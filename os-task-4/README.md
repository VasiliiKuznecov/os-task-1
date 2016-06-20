# os-task-4

Запуск:
`./blockingAccess.out passwd` - чтобы прочитать содержимое
`./blockingAccess.out passwd user password` - чтобы установить user пароль password

В программе специально сделаны задержки по 1 секунда после прочтения/записи каждой строки в файле passwd
Таким образом, можно запуситить сначала
`./blockingAccess.out passwd user password`
Потом, во 2ой консоли
`./blockingAccess.out passwd`
программа во 2ой консоли будет сообщать, что файл занят и, как только освободится, она его прочитает.
