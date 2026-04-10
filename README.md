# lab-9
## Таблица contacts
Поле -	Тип -	Ограничения -	Описание
id -	INTEGER -	PRIMARY KEY, AUTOINCREMENT -	Уникальный идентификатор контакта
name -	TEXT -	NOT NULL -	Имя контакта (обязательное поле)
email -	TEXT	-	Email адрес
phone -	TEXT	-	Номер телефона
address -	TEXT	-	Адрес проживания
category -	TEXT -	CHECK (friend, family, work) -	Категория контакта
created_at -	DATETIME -	DEFAULT CURRENT_TIMESTAMP -	Дата и время создани

## API протокол (TCP)  
Сервер работает на порту 12345. Все команды передаются в виде текстовых строк и заканчиваются символом \n.  
1. GET - Получение списка контактов  
Формат:  
text  
GET;категория;поиск;лимит  
Параметры:  
категория - фильтр по категории (friend, family, work, All)  
поиск - поиск по имени или email  
лимит - максимальное количество записей  
Пример:  
text  
GET;friend;Иван;100  
Ответ:  
text  
1;Иван Петров;ivan@mail.ru;+79991234567;Москва;friend  
2;Иван Иванов;ivan2@mail.ru;+79991112233;СПб;work  

2. ADD - Добавление контакта  
Формат:  
text  
ADD;имя;email;телефон;адрес;категория  
Пример:  
text  
ADD;Анна Смирнова;anna@mail.ru;+79995556677;Казань;family  
Ответ:  
text  
OK  

3. UPDATE - Обновление контакта  
Формат:  
text  
UPDATE;id;имя;email;телефон;адрес;категория  
Пример:  
text  
UPDATE;5;Анна Иванова;anna.new@mail.ru;+79991113333;Москва;work  
Ответ:  
text  
OK  

4. DELETE - Удаление контакта  
Формат:  
text  
DELETE;id  
Пример:  
text  
DELETE;5  
Ответ:  
text  
OK  

5. RESET - Сброс базы данных (опционально)  
Формат:  
text   
RESET  
Ответ:  
text  
OK  

## Примеры SQL запросов
1. CREATE TABLE - Создание таблицы
```
CREATE TABLE IF NOT EXISTS contacts (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    email TEXT,
    phone TEXT,
    address TEXT,
    category TEXT CHECK(category IN ('friend', 'family', 'work')),
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);
```
2. CREATE INDEX - Создание индексов
```
CREATE INDEX IF NOT EXISTS idx_contacts_category ON contacts(category);
CREATE INDEX IF NOT EXISTS idx_contacts_name ON contacts(name);
```
3. INSERT - Добавление контакта
```
-- Простой INSERT
INSERT INTO contacts (name, email, phone, address, category) 
VALUES ('Иван Петров', 'ivan@mail.ru', '+79991234567', 'Москва', 'friend');

-- С подготовленным запросом (prepared statement)
INSERT INTO contacts(name, email, phone, address, category) 
VALUES(?, ?, ?, ?, ?);
```
4. SELECT - Выборка контактов
```
-- Получить все контакты
SELECT * FROM contacts ORDER BY name;

-- Фильтр по категории
SELECT * FROM contacts WHERE category = 'friend';

-- Поиск по имени
SELECT * FROM contacts WHERE name LIKE '%Иван%';

-- Поиск по email
SELECT * FROM contacts WHERE email LIKE '%mail.ru%';

-- Комбинированный фильтр с сортировкой и ограничением
SELECT * FROM contacts 
WHERE category = 'work' AND name LIKE '%Анна%'
ORDER BY name ASC 
LIMIT 10;

-- Получить контакт по ID
SELECT * FROM contacts WHERE id = 5;
```
5. UPDATE - Обновление контакта
```
-- Обновление всех полей
UPDATE contacts 
SET name = 'Новое имя', 
    email = 'new@mail.ru',
    phone = '+79990001122',
    address = 'Новый адрес',
    category = 'family'
WHERE id = 5;

-- С подготовленным запросом
UPDATE contacts SET name=?, email=?, phone=?, address=?, category=? WHERE id=?;
```
6. DELETE - Удаление контакта
```
-- Удалить контакт по ID
DELETE FROM contacts WHERE id = 5;

-- С подготовленным запросом
DELETE FROM contacts WHERE id=?;

-- Удалить все контакты определённой категории
DELETE FROM contacts WHERE category = 'work';
```
