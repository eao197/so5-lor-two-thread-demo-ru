#include <so_5/all.hpp>

#include <random>

using namespace std::chrono_literals;

// Тип сообщения, которое будет отсылаться той нити, которая
// делает запись в файл.
struct write_data {
	const std::string file_name_;
};

// Сигнал, который отсылается нити чтения данных с датчика для
// уменьшения периода опроса.
struct dec_read_period : public so_5::signal_t {};

// Сигнал, который отсылается нити чтения данных с датчика для
// увеличения периода опроса.
struct inc_read_period : public so_5::signal_t {};

// Вспомогательная функция для конвертирования duration в ms.
template<typename R, typename P>
std::chrono::milliseconds to_ms(std::chrono::duration<R, P> v) {
	return std::chrono::duration_cast<std::chrono::milliseconds>(v);
}

// Нить чтения данных с датчика.
void meter_reader_thread(
		// Канал, который нужен для управления этой нитью.
		so_5::mchain_t control_ch,
		// Канал для сигналов acquisition_turn.
		so_5::mchain_t timer_ch,
		// Канал, в который будут отсылаться команды на запись файла.
		so_5::mchain_t file_write_ch) {

	// Границы для допустимого времени опроса.
	const auto period_left_range = to_ms(50ms);
	const auto period_right_range = to_ms(2s);

	// Текущий темп опроса.
	auto current_period = to_ms(300ms);

	// Тип для периодического сигнала от таймера.
	struct acquisition_turn : public so_5::signal_t {};

	// Просто счетчик чтений. Нужен для генерации новых имен файлов.
	int ordinal = 0;

	// Сразу же отсылаем себе первый сигнал на чтение данных.
	so_5::send<acquisition_turn>(timer_ch);

	// Читаем все из каналов до тех пор, пока каналы не закроют.
	so_5::select(so_5::from_all(),
		// Обработчик для сигналов от таймера.
		case_(timer_ch,
			// Этот обработчик будет вызван когда в канал попадет
			// сигнал типа acquire_turn.
			[&](so_5::mhood_t<acquisition_turn>) {
				// Нам потребуется узнать, сколько времени мы потратили на
				// всю операцию. Поэтому делаем засечку.
				const auto started_at = std::chrono::steady_clock::now();

				// Имитируем опрос датчика.
				std::cout << "meter read started" << std::endl;
				std::this_thread::sleep_for(50ms);
				std::cout << "meter read finished" << std::endl;

				// Отдаем команду на запись нового файла.
				so_5::send<write_data>(file_write_ch,
						"data_" + std::to_string(ordinal) + ".dat");
				++ordinal;

				// Теперь можем вычислить сколько же всего времени было
				// потрачено.
				const auto duration = std::chrono::steady_clock::now() - started_at;
				// Если потратили слишком много, то инициируем следующий
				// опрос сразу же.
				if(duration >= current_period) {
					std::cout << "period=" << current_period.count()
							<< "ms, no sleep" << std::endl;
					so_5::send<acquisition_turn>(timer_ch);
				}
				else {
					// В противном случае можем позволить себе немного "поспать".
					const auto sleep_time = to_ms(current_period - duration);
					std::cout << "period=" << current_period.count() << "ms, sleep="
							<< sleep_time.count() << "ms" << std::endl;
					so_5::send_delayed<acquisition_turn>(timer_ch,
							current_period - duration);
				}
			}),
		// Обработчик сигналов из управляющего канала.
		case_(control_ch,
			// Обрабатываем увеличение интервала опроса.
			[&](so_5::mhood_t<inc_read_period>) {
				current_period = std::min(to_ms(current_period * 1.5), period_right_range);
			},
			// Обрабатываем уменьшение интервала опроса.
			[&](so_5::mhood_t<dec_read_period>) {
				current_period = std::max(to_ms(current_period / 1.5), period_left_range);
			})
	);
}

// Нить, которая будет записывать файлы.
void file_writer_thread(
		// Канал из которого будут читаться команды на запись.
		so_5::mchain_t file_write_ch) {
	// Вспомогательные инструменты для генерации случайных значений.
	std::mt19937 rd_gen{std::random_device{}()};
	// Значения для задержки рабочей нити будут браться из
	// диапазона [295ms, 1s].
	std::uniform_int_distribution<int> rd_dist{295, 1000};

	// Читаем все из канала до тех пор, пока канал не закроют.
	// В этом случае произойдет автоматический выход из receive.
	receive(from(file_write_ch),
			// Этот обработчик будет вызван когда в канал попадет
			// сообщение типа write_data.
			[&](so_5::mhood_t<write_data> cmd) {
				// Выбираем случайную длительность операции "записи".
				const auto pause = rd_dist(rd_gen);
				// Имитируем запись в файл.
				std::cout << cmd->file_name_ << ": write started (pause:"
						<< pause << "ms)" << std::endl;
				std::this_thread::sleep_for(std::chrono::milliseconds{pause});
				std::cout << cmd->file_name_ << ": write finished" << std::endl;
			});
}

int main() {
	// Запускаем SObjectizer.
	so_5::wrapped_env_t sobj;

	// Объекты-нити создаем заранее специально для того...
	std::thread meter_reader, file_writer;
	// ...чтобы можно было создать этот объект joiner.
	// Именно он будет вызывать join() для нитей при выходе из
	// main. При этом не важно, по какой причине мы из main выходим:
	// из-за нормального завершения или из-за ошибки/исключения.
	auto joiner = so_5::auto_join(meter_reader, file_writer);

	// Создаем каналы, которые потребуются нашим рабочим нитям.
	// Управляющий канал для meter_reader_thread. Без каких-либо ограничений.
	auto control_ch = so_5::create_mchain(sobj);
	// Канал, который будет использоваться для отсылки acquisition_turn.
	auto timer_ch = so_5::create_mchain(control_ch->environment(),
			// Отводим место всего под одно сообщение.
			1,
			// Память под mchain выделяем сразу.
			so_5::mchain_props::memory_usage_t::preallocated,
			// Если место в mchain-е не освободилось даже после ожидания,
			// то игнорируем самое новое сообщение.
			so_5::mchain_props::overflow_reaction_t::drop_newest);
	// Канал для записи измерений будет ограничен по размеру, с паузой
	// при попытке записать в полный mchain и с выбрасыванием самых старых
	// команд, если канал даже после паузы не освободился.
	auto writer_ch = so_5::create_mchain(sobj,
			// Ждем освобождения места не более 300ms.
			300ms,
			// Ждать в mchain-е могут не более 2-х сообщений.
			2,
			// Память под mchain выделяем сразу.
			so_5::mchain_props::memory_usage_t::preallocated,
			// Если место в mchain-е не освободилось даже после ожидания,
			// то выбрасываем самое старое сообщение из mchain-а.
			so_5::mchain_props::overflow_reaction_t::remove_oldest);
	// Каналы должны быть автоматически закрыты при выходе из main.
	// Если этого не сделать, то рабочие нити продолжат висеть внутри
	// receive() и join() для них не завершится.
	auto closer = so_5::auto_close_drop_content(control_ch, timer_ch, writer_ch);

	// Теперь можно стартовать наши рабочие нити.
	meter_reader = std::thread(meter_reader_thread, control_ch, timer_ch, writer_ch);
	file_writer = std::thread(file_writer_thread, writer_ch);

	// Программа продолжит работать пока пользователь не введет exit или
	// пока не закроет стандартный поток ввода.
	bool stop_execution = false;
	while(!stop_execution) {
		std::cout << "Type 'exit' to quit, 'inc' or 'dec':" << std::endl;

		std::string cmd;
		if(std::getline(std::cin, cmd)) {
			if("exit" == cmd)
				stop_execution = true;
			else if("inc" == cmd)
				so_5::send<inc_read_period>(control_ch);
			else if("dec" == cmd)
				so_5::send<dec_read_period>(control_ch);
		}
		else
			stop_execution = true;
	}

	// Просто завершаем main. Все каналы будут закрыты автоматически
	// (благодаря объекту closer), все нити также завершаться автоматически
	// (благодаря объекту joiner).
	return 0;
}

