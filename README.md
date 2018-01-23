# Описание библиотеки FTCL

## Обозначения

**Time-запрос( time-сообщение )** - запрос на определенное время выполнения T, если в течении этого времени запрос не завершается, то запрос отменяется.

**Перезагрузка узла** - перезагрузка worker или мастера с помощью инструмента ipmitool. Перезагрузка осуществляется не более одного раза.

**Реинициализация воркера** -
1) пересоздание процесса worker в случае возврата кода ошибки из MPI функций( первая ошибка узла ).  
2) Перезагрузка узла в случае истечения времени ожидания окончания MPI функции или при повторном возврате кода ошибки из MPI функции.  
3) Перевод worker в состояние неактивного при возврате кода ошибки из MPI функции после перезагрузки узла.

## Схема работы

### 1. Инициализация  

**Воркер**: шлет мастеру time-сообщение, в случае неуспеха аварийно завершается( MPI_Abort )  

**Мастер**: ждет инициализационных сообщений от воркеров. Воркер становится активным, если отправил сообщение мастеру. Проверка сообщения инициализации воркера без времени( на всю работу программы )  
Пересылка сообщений инициализации.

![](/init.jpg)

Один из воркеров не смог корректно отослать сообщение инициализации и остался неактивным.

![](/init_with_fail.jpg)

### 2. Получение имен воркеров(опционально?) 

- **Мастер**: шлет time-запрос на получение имени воркера. В случае ошибки передачи, реинициализация воркера. 

**Воркер**: ждет запрос от мастера. Если ошибка ожидания(возврат кода ошибки от IProbe).
- **Воркер**: шлет time-сообщение мастеру. В случае неудачи смена мастера на запасного.

**Мастер**: принимает сообщение от worker'а. В случае неудачи реинициализация worker'а.

Отправка запросов на имена worker'ов. Один из запросов закончился с ошибкой.

![](workersName2.jpg)

Делаем реинициализацию worker'а.

![](getName3.jpg)

Для пересозданнаого worker'а переходим к пункту 1.

### 3. Работа

### 4. Завершение работы

- Мастер шлет time-запрос на завершение воркеру. В случае истечения времени - аварийное завершение через время T. В случае ошибки выполнения MPI функции - аварийное завершение.
- Ожидание ответа от воркера в течении времени T. Если ответа нет, то аварийное завершение.

В данный момент сделано:  
1. Класс NetworkModule - все операции передачи информации через сеть. Каждый метод защищен mutex’ом.  
2. Классы Logger - логирование действий в консоль и файл.  
3. Класс Master - мастер  
4. Класс Worker - воркер  
5. Класс Status - информация о воркерах, их состояние, все request и время работы неблокирующих операций MPI.  
6. Классы очередей 

Что надо сделать:   
1. Разобраться до конца с восстановлением состояния узлов  
2. Перезагрузка узлов  
3. Интерфейс задач  
4. Сериализация объектов - (пока думаю, сделать boost.serialization или что то свое)
