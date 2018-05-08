#include <so_5/all.hpp>

#include <random>

using namespace std::chrono_literals;

// Тип сообщения, которое будет отсылаться той нити, которая
// делает запись в файл.
struct write_data {
	const std::string file_name_;
};

// Нить чтения данных с датчика.
void meter_reader_thread(
		// Канал, который нужен этой нити.
		so_5::mchain_t timer_ch,
		// Канал, в который будут отсылаться команды на запись файла.
		so_5::mchain_t file_write_ch) {

	// Тип для периодического сигнала от таймера.
	struct acquisition_turn : public so_5::signal_t {};

	// Просто счетчик чтений. Нужен для генерации новых имен файлов.
	int ordinal = 0;

	// Запускаем таймер.
	// В этой версии у нас таймер срабатывает гораздо чаще.
	auto timer = so_5::send_periodic<acquisition_turn>(timer_ch, 0ms, 300ms);

	// Читаем все из канала до тех пор, пока канал не закроют.
	// В этом случае произойдет автоматический выход из receive.
	receive(from(timer_ch),
			// Этот обработчик будет вызван когда в канал попадет
			// сигнал типа acquire_turn.
			[&](so_5::mhood_t<acquisition_turn>) {
				// Имитируем опрос датчика.
				std::cout << "meter read started" << std::endl;
				std::this_thread::sleep_for(50ms);
				std::cout << "meter read finished" << std::endl;

				// Отдаем команду на запись нового файла.
				so_5::send<write_data>(file_write_ch,
						"data_" + std::to_string(ordinal) + ".dat");
				++ordinal;
			});
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
	// Канал для периодических сигналов будет ограничен по размеру,
	// без паузы при попытке записать в полный mchain и с выбрасыванием
	// самых новых сообщений.
	auto timer_ch = so_5::create_mchain(sobj,
			// Отводим место всего под одно сообщение.
			1,
			// Память под mchain выделяем сразу.
			so_5::mchain_props::memory_usage_t::preallocated,
			// Если канал полон, то самое новое сообщение игнорируется.	
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
	auto closer = so_5::auto_close_drop_content(timer_ch, writer_ch);

	// Теперь можно стартовать наши рабочие нити.
	meter_reader = std::thread(meter_reader_thread, timer_ch, writer_ch);
	file_writer = std::thread(file_writer_thread, writer_ch);

	// Программа продолжит работать пока пользователь не введет exit или
	// пока не закроет стандартный поток ввода.
	std::cout << "Type 'exit' to quit:" << std::endl;

	std::string cmd;
	while(std::getline(std::cin, cmd)) {
		if("exit" == cmd)
			break;
		else
			std::cout << "Type 'exit' to quit" << std::endl;
	}

	// Просто завершаем main. Все каналы будут закрыты автоматически
	// (благодаря объекту closer), все нити также завершаться автоматически
	// (благодаря объекту joiner).
	return 0;
}

