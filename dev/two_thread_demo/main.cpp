#include <so_5/all.hpp>

using namespace std::chrono_literals;

// Тип сообщения, которое будет отсылаться той нити, которая
// делает запись в файл.
struct write_data {
	const std::string file_name_;
};

// Нить чтения данных с датчика.
void meter_reader_thread(
		// Канал, который нужен этой нити.
		so_5::mchain_t self_ch,
		// Канал, в который будут отсылаться команды на запись файла.
		so_5::mchain_t file_write_ch) {

	// Тип для периодического сигнала от таймера.
	struct acquisition_turn : public so_5::signal_t {};

	// Просто счетчик чтений. Нужен для генерации новых имен файлов.
	int ordinal = 0;

	// Запускаем таймер.
	auto timer = so_5::send_periodic<acquisition_turn>(self_ch, 0ms, 750ms);

	// Читаем все из канала до тех пор, пока канал не закроют.
	// В этом случае произойдет автоматический выход из receive.
	receive(from(self_ch),
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
	// Читаем все из канала до тех пор, пока канал не закроют.
	// В этом случае произойдет автоматический выход из receive.
	receive(from(file_write_ch),
			// Этот обработчик будет вызван когда в канал попадет
			// сообщение типа write_data.
			[&](so_5::mhood_t<write_data> cmd) {
				// Имитируем запись в файл.
				std::cout << cmd->file_name_ << ": write started" << std::endl;
				std::this_thread::sleep_for(350ms);
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
	auto meter_ch = so_5::create_mchain(sobj);
	auto writer_ch = so_5::create_mchain(sobj);
	// Каналы должны быть автоматически закрыты при выходе из main.
	// Если этого не сделать, то рабочие нити продолжат висеть внутри
	// receive() и join() для них не завершится.
	auto closer = so_5::auto_close_drop_content(meter_ch, writer_ch);

	// Теперь можно стартовать наши рабочие нити.
	meter_reader = std::thread(meter_reader_thread, meter_ch, writer_ch);
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

