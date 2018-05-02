[TOC]

# Что это?
Это простой демонстрационный пример, который показывает, как с помощью
SObjectizer можно сделать так, чтобы одна рабочая нить периодически
выдавала команды второй рабочей нити.

# Как взять и попробовать?
Для экспериментов потребуется С++ компилятор с поддержкой C++14.

Есть два способа попробовать самостоятельно скомпилировать примеры: с помощью CMake и с помощью MxxRu.

## Компиляция с помощью CMake
Для компиляции примеров с помощью CMake требуется загрузить архив с именем вида so5_lor_two_thread_demo_ru-*-full.zip из секции [Downloads](https://bitbucket.org/sobjectizerteam/so5_lor_two_thread_demo_ru/downloads/). После чего:

~~~~~{.sh}
unzip so5_lor_two_thread_demo_ru-201805021000-full.zip
cd so5_lor_two_thread_demo_ru
mkdir cmake_build
cd cmake_build
cmake  -DCMAKE_INSTALL_PREFIX=target -DCMAKE_BUILD_TYPE=Release ../dev
cmake --build . --config Release --target install
~~~~~

Возможно, потребуется дополнительно указать название нужного вам тулчейна через опцию -G. Например:

~~~~~{.sh}
cmake  -DCMAKE_INSTALL_PREFIX=target -DCMAKE_BUILD_TYPE=Release -G "NMake Makefiles" ../dev
~~~~~

## Компиляция с помощью MxxRu
Для компиляции с помощью MxxRu потребуется Ruby и RubyGems (как правило, RubyGems устанавливается вместе с Ruby, но если это не так, то RubyGems нужно поставить явно). Для установки MxxRu нужно выполнить команду:

~~~~~{.sh}
gem install Mxx_ru
~~~~~

После того как Ruby, RubyGems и MxxRu установлены можно брать примеры непосредственно из Hg-репозитория:

~~~~~{.sh}
hg clone https://bitbucket.org/sobjectizerteam/so5_lor_two_thread_demo_ru
cd so5_lor_two_thread_demo_ru
mxxruexternals
cd dev
ruby build.rb
~~~~~

Либо же можно загрузить архив с именем вида so5_lor_two_thread_demo_ru-*-full.zip из секции [Downloads](https://bitbucket.org/sobjectizerteam/so5_lor_two_thread_demo_ru/downloads/). После чего:

~~~~~{.sh}
unzip so5_lor_two_thread_demo_ru-201805021000-full.zip
cd so5_lor_two_thread_demo_ru/dev
ruby build.rb
~~~~~

Скомпилированные примеры должны оказаться внутри подкаталога `target/release`.

Также перед сборкой может быть полезно выполнить:

~~~~~{.sh}
cp local-build.rb.example local-build.rb
~~~~~

и нужным вам образом отредактировать содержимое `local-build.rb`.
