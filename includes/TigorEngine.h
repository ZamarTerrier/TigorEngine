#ifndef TigorEngine_H
#define TigorEngine_H

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
void TEngineInitSystem(int width, int height, const char* name);
/// @brief Назначить текущий файл шрифтов
/// @param font_path - путь к шрифту
void TEngineSetFont(char *font_path);
/// @brief Функция установки рендера
/// @param obj - объект типа RenderTexture
/// @param count - количество объектов
void TEngineSetRender(void *obj, uint32_t count);
/// @brief Установка функции вызываемая при изменении окна
/// @param func - вызываемая функция
void TEngineSetRecreateFunc(RecreateFunc_T func);
/// @brief Функция для добавления объекта в очередь отрисовки
/// @param go - объект отрисовки
void TEngineDraw(GameObject *go);
/// @brief Вызов цикла отрисовки оъектов
///     Будет вызвана отрисовка указанных через TEngineDraw объектов на все указанные через EngineSetRender рендеры
void TEngineRender();
/// @brief Функция обработки событий окна
void TEnginePoolEvents();
/// @brief Установка обратного вызова событий кнопок клавиатуры
/// @param callback - Функция, которая будет вызвана при событии нажатия кнопок клавиатуры
void TEngineSetKeyCallback(void *callback);
/// @brief Установка обратного вызова ввода с клавиатуры
/// @param callback - Функция, которая будет вызвана при событии нажатия кнопок клавиатуры
void TEngineSetCharCallback(void *callback);
/// @brief Установка обратного вызова событий кнопок мыши
/// @param callback - Функция, которая будет вызвана при событии нажатия кнопок мыши
void TEngineSetMouseKeyCallback(void *callback);
/// @brief Установка обратного вызова при изменении позиции мыши на экране
/// @param callback - Функция вызываемая при изменении положения мыши
void TEngineSetCursorPoscallback(void * callback);
/// @brief Получени размеров окна
/// @param width - ширина окна
/// @param height - высота окна
void TEngineGetWindowSize(int *width, int *height);
/// @brief Фиксация указателя в центре экрана
void TEngineFixedCursorCenter();
/// @brief Получение положения указателя
/// @param xpos - положение по Х
/// @param ypos - положение по У
void TEngineGetCursorPos(double *xpos, double *ypos);
/// @brief Установка положения указателя
/// @param xpos - положение по Х
/// @param ypos - положение по У
void TEngineSetCursorPos(float xpos, float ypos);
/// @brief Состояние указателя
/// @param state - Состояние 1 : Отключен и не видим. Состояние 2 : Не видим и активен. Состояние 3 : Видим и активен
void TEngineHideCursor(char state);
/// @brief Нажата ли клавиша мыши
/// @param Key - Идентификатор мыши
/// @return Возвращет 0 (false) или 1 (true)
int TEngineGetMousePress(int Key);
/// @brief Нажата ли кнопка клавиатуры
/// @param Key - Идентификатор кнопки клавиатуры
/// @return Возвращет 0 (false) или 1 (true)
int TEngineGetKeyPress(int Key);
/// @brief Проверка закрыто ли окно
/// @return Возвращает 0 (false) и 1 (true)
int TEngineWindowIsClosed();
/// @brief Получение текущего времени с момента старта программы
/// @return Возвращает время
double TEngineGetTime();
/// @brief Получение строки сохраненной в буфере окна
/// @return Возвращает указатель на строку
const char *TEngineGetClipBoardString();
/// @brief Установка набора символов в буфере окна
/// @param string - Строка содежащая символы
void TEngineSetClipBoardString(const char *string);
/// @brief Очистка движка перед закрытием 
void TEngineCleanUp();

#ifdef __cplusplus
}
#endif


#endif