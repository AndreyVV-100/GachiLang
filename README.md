# GachiLang

This is a gachimuchi programming language.

## ♂Oh shit, I'm sorry♂

Now there is only Russian version of README. An English version will be written later.

## Введение

Эта программа была написана в рамках годового курса информатики на ФРКТ МФТИ в 2020-2021 годах. Если вы нашли какие-то ошибки, или у вас есть предложения по улучшению компилятора, то свяжитесь со мной (см. контакты профиля).

## Как это работает?

Программа разделена на 2 основные части: ```Frontend``` и  ```Backend```, так же в папке ```Tree``` есть вспомогательные файлы: ```Tree``` и ```Stack``` (```.cpp``` и ```.h```), которые являются основой для них. В ```Stack``` написана структура стека и функции для работы с ним, в нём хранятся все элементы (зачем - будет понятно понятно позднее), а в ```Tree``` - функционал дерева, его графический dump и сохранение/загрузка из файла. Подробнее о вспомогательных файлах мы говорить не будем, их реализация схожа на реализацию в других проектах (см. репозитории аккаунта ```Stack-not-overflow```, ```Akinator```, ```Differeciator```).

### Frontend

Эта программа превращает исходный код программы в дерево. Она состоит из двух частей: лексический анализ (```Lexical Analyze```) и лексический разбор (```Lexical Parse```). Первая из них просто разбивает текст на лексемы и создаёт узлы, которые после будут связаны в единое дерево. Вторая же, следуя правилам языка, создаёт их. Этот процесс очень схож с работой калькулятора (см. репозиторий ```Calculator```). Здесь же только приведём правила, по которым осуществляется разбор:

```
Gr ::= {Func}* Func(sex) {Func}*
Func ::= Ind FuncParam Body
FuncParam ::= "ass " Ind* " ass"

Body ::= Oper | "AAAAH" Oper+ "AAAAH"
Oper ::= {Ar | Call | Ret}$ | Cond

Ar ::= Ind '=' Exp
Exp ::= E
// E see in calculator, except:
P ::= (E) | Ind | N | Call

Call ::= Ind CallParam
CallParam ::= "ass " {Ind | N}* " ass"

Cond ::= CondKey E Comp E Body
CondKey ::= "♂fantasies♂" | "♂let's go♂"
Comp ::= "<" | ">" | "<=" | ">=" | "!=" | "=="

Ret ::= "♂next door♂" E
Ind ::= '♂' {a-z A-Z <space> _ 0-9} '♂'
```

### Backend

Эта программа создаёт на основе дерева elf-файл (архитектура x86-64). Для этого производится обход дерева и распечатка байт, соответствующих командам процессора. Отметим следующие особенности компилятора:

1) Заголовок файла был написан заранее, как и большая часть сегмента памяти ```.text``` (он всего один, т.к. потребности в других сегментах нет), там же только подставляются размеры сегмента. Поэтому elf-файл формируется на основе "заготовки" ```Backend/ServiseFiles/MagicHeader```. После формирования заголовков выполняется выравнивание по 0x1000 байт, а затем записывается сам исполняемый код. Точка входа всегда является началом сегмента ```.text``` (т.е. 0x1000), где сразу же программа вызывает функцию ♂sex♂ (аналог main).

2) В программе есть 3 доступные функции: ```♂spanking♂``` (sqrt), ```♂cumming♂``` (printf), ```♂swallow♂``` (scanf). И если первая является ассемблерной командой, которую достаточно просто подставить в байт-код, то 2 другие далеко не так тривиальны, поэтому они были написаны отдельно на языке ```assembly```, и записываются в байт-код во время записи адресов команд ```call``` (аналог линковки). Для этого созданы файлы ```.gcmlib``` в папке ```Libraries```.

3) Во время вычислений симулируется стек на ```xmm-регистрах```, для этого заведёт отдельный счётчик. Поэтому глубина вложенности вычислений не может превышать 16, но для одной команды это является слишком большой вложенностью, поэтому в случае переполнения автору рекомендуется переписать код.

4) Размер стекового фрейма фиксирован - 128 байт, поэтому программа поддерживает не более 15 локальных переменных в одной функции. В случае возможного превышения этого числа программа "кидает" ```warning```, хотя, скорее всего, сформированный elf-файл будет неисправен.

### Middle end

Возможно, здесь что-нибудь когда-нибудь будет, действия этой программы будут схожи с действиями оптимизатора дерева (см. репозиторий ```Differenciator```). Ещё можно воспользоваться инструментарием ```LLVM```, но это уже далёкая перспектива развития...

### Немножко о предыдущих версиях

До этого программа пользовалась языком ассемблера нашего собственного процессора (см. репозиторий ```Processor```), теперь эта версия выделена в отдельную ветвь, но далее она развиваться не будет, т.к. виртуальный процессор слишком медленный по сравнению с нативным кодом. Так, например, для вычисления корней уравнения ```x^2 + 300x + 8 = 0``` 10 миллионов раз виртуальному процессору ```1488``` секунд, а исполняемому файлу - ```228``` секунд. Таким образом, ускорение составило ```300``` раз.

### Как запустить компилятор?

Все действия описаны здесь для ОС Linux.

Скопируйте репозиторий в какую-нибудь папку:

```Terminal
git clone https://github.com/AndreyVV-100/GachiLang.git
```

После зайдите в неё и скомпилируйте проект (для этого используется компилятор ```g++```):

```Terminal
make
```

После чего вы можете компилировать файлы, их имя указывается в качестве первого параметра:

```Terminal
./gcm filename
```

Если он не указан, то компилируется Examples/SqrtTripleDick.gcm

## Спасибо за внимание!
