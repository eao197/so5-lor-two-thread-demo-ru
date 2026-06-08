# Что это?
Это простой демонстрационный пример, который показывает, как с помощью
SObjectizer можно сделать так, чтобы одна рабочая нить периодически
выдавала команды второй рабочей нити без использования агентов, только
посредством mchain-ов.

# Как взять и попробовать?
Для экспериментов потребуется С++ компилятор с поддержкой C++17 и vcpkg.

Допустим, vcpkg установлен в `~/vcpkg`:

```sh
git clone https://github.com/eao197/so5-lor-two-thread-demo-ru
cd so5-lor-two-thread-demo-ru
mkdir cmake_build
cd cmake_build
VCPKG_HOME=~/vcpkg cmake  -DCMAKE_INSTALL_PREFIX=target -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release --target install
```

Возможно, потребуется дополнительно указать название нужного вам тулчейна через опцию -G. Например:

```sh
cmake  -DCMAKE_INSTALL_PREFIX=target -DCMAKE_BUILD_TYPE=Release -G "NMake Makefiles" ..
```

