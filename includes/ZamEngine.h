#ifndef ZAMENGINE_H
#define ZAMENGINE_H

#include <stdint.h>

#include <Objects/gameObject.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef void (*DrawFunc_T)();
typedef void (*RecreateFunc_T)();

/// @brief Инициализация Движка
/// @param width - Ширина экрана
/// @param height - Высота экрана
/// @param name - Имя программы
void ZEngineInitSystem(int width, int height, const char* name);
/// @brief Назначить текущий файл шрифтов
/// @param font_path - путь к шрифту
void ZEngineSetFont(char *font_path);
/// @brief Функция установки рендера
/// @param obj - объект типа RenderTexture
/// @param count - количество объектов
void ZEngineSetRender(void *obj, uint32_t count);
/// @brief Установка функции вызываемая при изменении окна
/// @param func - вызываемая функция
void ZEngineSetRecreateFunc(RecreateFunc_T func);
/// @brief Функция для добавления объекта в очередь отрисовки
/// @param go - объект отрисовки
void ZEngineDraw(GameObject *go);
/// @brief Вызов цикла отрисовки оъектов
///     Будет вызвана отрисовка указанных через ZEngineDraw объектов на все указанные через EngineSetRender рендеры
void ZEngineRender();
/// @brief Функция обработки событий окна
void ZEnginePoolEvents();
/// @brief Установка обратного вызова событий кнопок клавиатуры
/// @param callback - Функция, которая будет вызвана при событии нажатия кнопок клавиатуры
void ZEngineSetKeyCallback(void *callback);
/// @brief Установка обратного вызова ввода с клавиатуры
/// @param callback - Функция, которая будет вызвана при событии нажатия кнопок клавиатуры
void ZEngineSetCharCallback(void *callback);
/// @brief Установка обратного вызова событий кнопок мыши
/// @param callback - Функция, которая будет вызвана при событии нажатия кнопок мыши
void ZEngineSetMouseKeyCallback(void *callback);
/// @brief Установка обратного вызова при изменении позиции мыши на экране
/// @param callback - Функция вызываемая при изменении положения мыши
void ZEngineSetCursorPoscallback(void * callback);
/// @brief Получени размеров окна
/// @param width - ширина окна
/// @param height - высота окна
void ZEngineGetWindowSize(int *width, int *height);
/// @brief Фиксация указателя в центре экрана
void ZEngineFixedCursorCenter();
/// @brief Получение положения указателя
/// @param xpos - положение по Х
/// @param ypos - положение по У
void ZEngineGetCursorPos(double *xpos, double *ypos);
/// @brief Установка положения указателя
/// @param xpos - положение по Х
/// @param ypos - положение по У
void ZEngineSetCursorPos(float xpos, float ypos);
/// @brief Состояние указателя
/// @param state - Состояние 1 : Отключен и не видим. Состояние 2 : Не видим и активен. Состояние 3 : Видим и активен
void ZEngineHideCursor(char state);
/// @brief Нажата ли клавиша мыши
/// @param Key - Идентификатор мыши
/// @return Возвращет 0 (false) или 1 (true)
int ZEngineGetMousePress(int Key);
/// @brief Нажата ли кнопка клавиатуры
/// @param Key - Идентификатор кнопки клавиатуры
/// @return Возвращет 0 (false) или 1 (true)
int ZEngineGetKeyPress(int Key);
/// @brief Проверка закрыто ли окно
/// @return Возвращает 0 (false) и 1 (true)
int ZEngineWindowIsClosed();
/// @brief Получение текущего времени с момента старта программы
/// @return Возвращает время
double ZEngineGetTime();
/// @brief Получение строки сохраненной в буфере окна
/// @return Возвращает указатель на строку
const char *ZEngineGetClipBoard();
/// @brief Установка набора символов в буфере окна
/// @param string - Строка содежащая символы
void ZEngineSetClipBoardString(const char *string);
/// @brief Очистка движка перед закрытием 
void ZEngineCleanUp();

#ifdef __cplusplus
}
#endif


#endif