/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
Севастопольский государственный университет
Кафедра «Информационные технологии и системы»

Программа для работы с базой данных учёных
Текст программы
РАЗРАБОТАЛ
Студент гр. ИИ/б-25-4-о
Гамаюнов М.А.
2026
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
Программа работает с базой данных об ученых, которые загружаются из файла
или создаются в виде новой таблицы. Каждая строка файла содержит запись
об одном ученом для которого отводится 32 символа с учетом \0, под указание
фамилии, имени, отчества, научной области, учёной степени, количество
статей, индекс Хирша, количество цитирований.
Основные функции программы: 
-       создание новой таблицы с очищением текущей и добавление новых записей;
-	вывод базы данных на экран;
-	удаление записи из таблицы;
-	изменение записей по элементам в базе; 
-	сортировка базы по возрастанию и убыванию; 
-	поиск записей об ученых в базе по конкретному полю; 
-       сохранение и загрузка базы данных в текстовом и бинарном видах;
-	вывод топ 5 ученых по индексу Хирша и топ 5 по количеству цитирований
в каждой из учёных областей.
Вариант задания 4. Утверждено 18.02.2026 Среда программирования Neovim v0.11.5
 Дата последней коррекции: 30.05.2026.
 Версия 1.0 
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <wchar.h>
#include <fcntl.h> 
#include <wctype.h>

/*-----------------Коды для клавиш, чтобы не путаться в коде------------------*/
#define KEY_UP     1001
#define KEY_DOWN   1002
#define KEY_LEFT   1003
#define KEY_RIGHT  1004
#define KEY_ENTER  10
#define KEY_ESC    27

/*----------------------------Фиксируем ширину окон---------------------------*/
#define W         54
#define WIDE      121

/*------------------------------Структура учёного-----------------------------*/
typedef struct {
    int id;
    wchar_t fam[32];
    wchar_t im[32];
    wchar_t ot[32];
    wchar_t obl[32];
    wchar_t step[32];
    int stat;
    int hirsh;
    int cit;
} Scientist;

/*----------------------------Двухсвязный список------------------------------*/
typedef struct List {
    Scientist data;
    struct List *prev;
    struct List *next;
} List;

const int SZ = sizeof(Scientist);

/*---------------Сообщаем компилятору о существании функций и их возвращаемом типе
и параметрах заранее----------------------------------------------------------*/
void ClearBuf(void);
wchar_t ToLow(wchar_t c);
wchar_t ToUp(wchar_t c);
int CmpStr(const wchar_t *a, const wchar_t *b);
void PrintTop(int w);
void PrintBottom(int w);
void PrintCenter(const wchar_t *txt, int w);
void ErrorBox(const wchar_t *msg);
void GetFileName(char *s, int n);
int GetKey(void);
void ShortenString(wchar_t *s, int max);
int ValidText(wchar_t *s);
void GetStr(wchar_t *s, int max, const wchar_t *name, int onlyAlpha);
int GetInt(const wchar_t *name);
int GetRange(const wchar_t *prompt, int *lo, int *hi);
int TotalPages(List *lst);
void PrintCentered(const wchar_t *txt, int w);
void BlueBtn(const wchar_t *txt, int w);
void PrintMenuItem(const char *txt, int sel);
void PrintHelp(const wchar_t *txt, int w);
void DrawMenu(int sel, const char *items[], int cnt, const wchar_t *title);
void PrintTableHead(void);
void PrintTableBott(void);
void PrintDivider(void);
List *PrintPage(List *lst, int p, int total);
List *NextPage(List *lst);
List *PrevPage(List *lst);
Scientist ReadScientist(void);
void PrintScientist(Scientist a);
List *FindById(List *lst, int id);
List *AddFirst(List *lst, Scientist a);
List *AddLast(List *lst, Scientist a);
void FreeList(List *lst);
int NextId(List *lst);
int PickField(void);
void QuickSortLow(List *left, List *right, int (*cmp)(const Scientist*, const Scientist*));
void QuickSortHigh(List *left, List *right, int (*cmp)(const Scientist*, const Scientist*));
List *SortLow(List *lst, int (*cmp)(const Scientist*, const Scientist*));
List *SortHigh(List *lst, int (*cmp)(const Scientist*, const Scientist*));
int CmpName(const Scientist *a, const Scientist *b);
int CmpField(const Scientist *a, const Scientist *b);
int CmpDegree(const Scientist *a, const Scientist *b);
int CmpArticles(const Scientist *a, const Scientist *b);
int CmpHIndex(const Scientist *a, const Scientist *b);
int CmpCitations(const Scientist *a, const Scientist *b);
void SwapSci(Scientist *a, Scientist *b);
void ViewSearchResults(List *results, const wchar_t *title);
void ShowResult(List *lst, int type, const wchar_t *strVal, int num);
void ShowRange(List *lst, int type, int lo, int hi);
void CollectUnique(List *lst, int type, wchar_t vals[][32], int *cnt);
int PickValue(wchar_t vals[][32], int cnt, wchar_t *out);
void FindName(List *lst);
void FindField(List *lst);
void FindDegree(List *lst);
void FindArticles(List *lst);
void FindHIndex(List *lst);
void FindCitations(List *lst);
void ExitScreen(void);
int SaveTxt(List *lst);
int SaveBin(List *lst);
int CheckTxtFile(FILE *f);
int CheckBinFile(FILE *f);
List *LoadTxt(List *lst);
List *LoadBin(List *lst);
void ShowErrorAndExit(const wchar_t *msg);
List *CreateTable(List *lst);
List *CreateAdd(List *lst);
void ViewTable(List *lst);
List *DeleteById(List *lst);
void EditRecord(List *lst);
List *SortTable(List *lst);
void SearchRecord(List *lst);
void SaveMenu(List *lst);
List *LoadMenu(List *lst);
void TopFive(List *lst);
void ExitProg(List *lst);

/*-----------------------------Реализация функций-----------------------------*/

void ClearBuf(void) {
    int c;
    while ((c = getwchar()) != L'\n' && c != WEOF) { ; }
}

wchar_t ToLow(wchar_t c) {
    if (c >= L'А' && c <= L'Я') return c + (L'а' - L'А');
    if (c == L'Ё') return L'ё';
    if (c >= L'A' && c <= L'Z') return c + (L'a' - L'A');
    return c;
}

wchar_t ToUp(wchar_t c) {
    if (c >= L'а' && c <= L'я') return c - (L'а' - L'А');
    if (c == L'ё') return L'Ё';
    if (c >= L'a' && c <= L'z') return c - (L'a' - L'A');
    return c;
}

int CmpStr(const wchar_t *a, const wchar_t *b) {
    while (*a && *b) {
        if (ToLow(*a) != ToLow(*b))
            return ToLow(*a) - ToLow(*b);
        a++; b++;
    }
    return ToLow(*a) - ToLow(*b);
}

void PrintTop(int w) {
    wprintf(L"┌");
    for (int i = 0; i < w - 2; i++) wprintf(L"─");
    wprintf(L"┐\n");
}

void PrintBottom(int w) {
    wprintf(L"└");
    for (int i = 0; i < w - 2; i++) wprintf(L"─");
    wprintf(L"┘\n");
}

void PrintCenter(const wchar_t *txt, int w) {
    int len = wcslen(txt);
    int pad = (w - 2 - len) / 2;
    int extra = (w - 2 - len) % 2;
    if (pad < 0) pad = 0;
    wprintf(L"│");
    for (int i = 0; i < pad; i++) wprintf(L" ");
    wprintf(L"%.*ls", w - 2, txt);
    for (int i = 0; i < pad + extra; i++) wprintf(L" ");
    wprintf(L"│\n");
}

void ErrorBox(const wchar_t *msg) {
    wprintf(L"\n");
    PrintTop(W);
    PrintCenter(L"ОШИБКА ВВОДА", W);
    PrintCenter(msg, W);
    PrintBottom(W);
}

void GetFileName(char *s, int n) {
    if (fgets(s, n, stdin)) {
        size_t len = strlen(s);
        if (len > 0 && s[len - 1] == '\n') s[len - 1] = '\0';
    }
}

int GetKey(void) {
    struct termios old, new;
    int ch;
    tcgetattr(STDIN_FILENO, &old);
    new = old;
    new.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new);
    ch = getchar();
    if (ch == 27) {
        int a = -1, b = -1;
        int fl = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, fl | O_NONBLOCK);
        a = getchar();
        if (a == '[') {
            b = getchar();
            if (b == 'A') ch = KEY_UP;
            else if (b == 'B') ch = KEY_DOWN;
            else if (b == 'C') ch = KEY_RIGHT;
            else if (b == 'D') ch = KEY_LEFT;
        }
        fcntl(STDIN_FILENO, F_SETFL, fl);
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &old);
    return ch;
}

void ShortenString(wchar_t *s, int max) {
    int len = wcslen(s);
    if (len <= max) return;
    int spaces = 0;
    for (int i = 0; i < len; i++) if (s[i] == L' ') spaces++;
    if (spaces > 0) {
        wchar_t tmp[100];
        wcscpy(tmp, s);
        int last = -1;
        for (int i = wcslen(tmp) - 1; i >= 0; i--) {
            if (tmp[i] == L' ') { last = i; break; }
        }
        if (last != -1 && last + 1 < wcslen(tmp)) {
            tmp[last + 2] = L'.';
            tmp[last + 3] = L'\0';
        }
        if (wcslen(tmp) <= max) { wcscpy(s, tmp); return; }
        wcscpy(tmp, s);
        int cnt = 0, first = -1;
        for (int i = 0; i < wcslen(tmp); i++) {
            if (tmp[i] == L' ') {
                cnt++;
                if (cnt == 1) { first = i; break; }
            }
        }
        if (first != -1) {
            wchar_t c2 = tmp[first + 1];
            wchar_t c3 = L'\0';
            int sec = -1;
            for (int i = first + 1; i < wcslen(tmp); i++) {
                if (tmp[i] == L' ') { sec = i; break; }
            }
            if (sec != -1 && sec + 1 < wcslen(tmp)) c3 = tmp[sec + 1];
            int idx = first + 1;
            if (c2 != L'\0' && c2 != L' ') {
                tmp[idx] = c2; idx++;
                tmp[idx] = L'.'; idx++;
            }
            if (c3 != L'\0' && c3 != L' ') {
                tmp[idx] = c3; idx++;
                tmp[idx] = L'.'; idx++;
            }
            tmp[idx] = L'\0';
            if (wcslen(tmp) <= max) { wcscpy(s, tmp); return; }
        }
    }
    s[max - 3] = L'.';
    s[max - 2] = L'.';
    s[max - 1] = L'.';
    s[max] = L'\0';
}

int ValidText(wchar_t *s) {
    wchar_t *p = s;
    while (*p == L' ') p++;
    if (*p == L'\0') return 0;
    wchar_t *e = s + wcslen(s) - 1;
    while (e > p && *e == L' ') e--;
    *(e + 1) = L'\0';
    if (p != s) wmemmove(s, p, (e - p + 2) * sizeof(wchar_t));
    int len = wcslen(s);
    int lastSp = 0, lastHyp = 0;
    for (int i = 0; i < len; i++) {
        wchar_t c = s[i];
        if (c == L' ') {
            if (lastSp || i == 0 || i == len - 1) return 0;
            lastSp = 1; lastHyp = 0;
        } else if (c == L'-') {
            if (lastHyp || i == 0 || i == len - 1) return 0;
            lastHyp = 1; lastSp = 0;
        } else if (iswalpha(c)) {
            lastHyp = 0; lastSp = 0;
        } else {
            return 0;
        }
    }
    int newWord = 1;
    for (int i = 0; i < wcslen(s); i++) {
        if (s[i] == L' ' || s[i] == L'-') newWord = 1;
        else if (newWord) {
            s[i] = ToUp(s[i]);
            newWord = 0;
        } else {
            s[i] = ToLow(s[i]);
        }
    }
    return 1;
}

void GetStr(wchar_t *s, int max, const wchar_t *name, int onlyAlpha) {
    wchar_t buf[200];
    while (1) {
        wprintf(L"Введите %ls: ", name);
        if (!fgetws(buf, 200, stdin)) continue;
        size_t len = wcslen(buf);
        if (len > 0 && buf[len - 1] == L'\n') { buf[len - 1] = L'\0'; len--; }
        if (len == 0) { ErrorBox(L"Поле не может быть пустым!"); continue; }
        if (len >= (size_t)max) { ErrorBox(L"Введенное поле выше допустимого"); continue; }
        if (onlyAlpha) {
            wchar_t tmp[200];
            wcscpy(tmp, buf);
            if (!ValidText(tmp)) {
                ErrorBox(L"Введите буквы (допустим ввод через дефис)");
                continue;
            }
            wcscpy(buf, tmp);
        }
        wcscpy(s, buf);
        break;
    }
}

int GetInt(const wchar_t *name) {
    wchar_t buf[100];
    long long val;
    while (1) {
        wprintf(L"Введите %ls: ", name);
        if (!fgetws(buf, 100, stdin)) continue;
        size_t len = wcslen(buf);
        if (len > 0 && buf[len - 1] == L'\n') { buf[len - 1] = L'\0'; len--; }
        if (len == 0) { ErrorBox(L"Поле не может быть пустым"); continue; }
        int ok = 1;
        for (size_t i = 0; i < len; i++) if (!iswdigit(buf[i])) { ok = 0; break; }
        if (!ok) { ErrorBox(L"Введите значение"); continue; }
        if (swscanf(buf, L"%lld", &val) != 1) { ErrorBox(L"Ошибка распознавания числа"); continue; }
        if (val > 999999) { ErrorBox(L"Введенное поле выше допустимого"); continue; }
        return (int)val;
    }
}

int GetRange(const wchar_t *prompt, int *lo, int *hi) {
    wprintf(L"%ls\n", prompt);
    *lo = GetInt(L"нижнюю границу диапазона");
    *hi = GetInt(L"верхнюю границу диапазона");
    if (*lo > *hi) {
        ErrorBox(L"Нижняя граница не может быть больше верхней");
        return 0;
    }
    return 1;
}

int TotalPages(List *lst) {
    if (!lst) return 0;
    List *t = lst;
    while (t->prev) t = t->prev;
    int cnt = 0;
    while (t) { cnt++; t = t->next; }
    int pages = cnt / 10;
    if (cnt % 10 != 0 || pages == 0) pages++;
    return pages;
}

void PrintCentered(const wchar_t *txt, int w) {
    int len = wcslen(txt);
    int pad = (w - len) / 2;
    for (int i = 0; i < pad; i++) wprintf(L" ");
    wprintf(L"%ls", txt);
    for (int i = 0; i < w - len - pad; i++) wprintf(L" ");
}

void BlueBtn(const wchar_t *txt, int w) {
    int left = 2, right = 2;
    int inner = w - left - right;
    int tlen = wcslen(txt);
    int tot = tlen + 6;
    int sp = inner - tot;
    int lsp = sp / 2;
    int rsp = sp - lsp;
    for (int i = 0; i < left; i++) wprintf(L" ");
    wprintf(L"\033[97;44m");
    wprintf(L" > ");
    for (int i = 0; i < lsp; i++) wprintf(L" ");
    wprintf(L"%ls", txt);
    for (int i = 0; i < rsp; i++) wprintf(L" ");
    wprintf(L" < ");
    wprintf(L"\033[0m");
    for (int i = 0; i < right; i++) wprintf(L" ");
    wprintf(L"\n");
}

void PrintMenuItem(const char *txt, int sel) {
    if (sel) {
        wprintf(L"│  \033[97;44m > %-42s < \033[0m  │\n", txt);
    } else {
        wprintf(L"│     %-42s     │\n", txt);
    }
}

void PrintHelp(const wchar_t *txt, int w) {
    int len = wcslen(txt);
    wprintf(L"│ %ls", txt);
    int fill = w - 2 - 1 - len;
    for (int i = 0; i < fill; i++) wprintf(L" ");
    wprintf(L"│\n");
}

void DrawMenu(int sel, const char *items[], int cnt, const wchar_t *title) {
    printf("\033[2J\033[H");
    PrintTop(W);
    PrintCenter(title, W);
    PrintBottom(W);
    wprintf(L"\n");
    PrintTop(W);
    for (int i = 0; i < cnt; i++) PrintMenuItem(items[i], i == sel);
    PrintBottom(W);
    wprintf(L"\n");
    PrintTop(W);
    PrintHelp(L"Стрелки Вверх/Вниз - Листать меню", W);
    PrintHelp(L"Enter              - Выбрать действие", W);
    PrintHelp(L"ESC                - Назад / Выход", W);
    PrintBottom(W);
}

void PrintTableHead(void) {
    wprintf(L"┌────┬────────────────────────────────────────┬─────────────────┬────────────────────┬────────────┬──────────┬──────────┐\n");
    wprintf(L"│ ID │ ФИО                                    │ Область         │ Учёная степень     │  Статей    │  Хирш    │  Цитир   │\n");
    wprintf(L"├────┼────────────────────────────────────────┼─────────────────┼────────────────────┼────────────┼──────────┼──────────┤\n");
}

void PrintTableBott(void) {
    wprintf(L"└────┴────────────────────────────────────────┴─────────────────┴────────────────────┴────────────┴──────────┴──────────┘\n");
}

void PrintDivider(void) {
    wprintf(L"├────┼────────────────────────────────────────┼─────────────────┼────────────────────┼────────────┼──────────┼──────────┤\n");
}

List *PrintPage(List *lst, int p, int total) {
    if (!lst) return NULL;
    PrintTableHead();
    List *t = lst;
    wchar_t fio[100], obl[32], step[32];
    int rows = 0;
    for (int i = 0; t && i < 10; i++, t = t->next) {
        swprintf(fio, 100, L"%ls %ls %ls", t->data.fam, t->data.im, t->data.ot);
        wcscpy(obl, t->data.obl);
        wcscpy(step, t->data.step);
        ShortenString(fio, 38);
        ShortenString(obl, 15);
        ShortenString(step, 18);
        wprintf(L"│%3d │ %-38ls │ %-15ls │ %-18ls │ %10d │ %8d │ %8d │\n",
                t->data.id, fio, obl, step, t->data.stat, t->data.hirsh, t->data.cit);
        rows++;
        if (t->next && rows < 10) PrintDivider();
    }
    PrintTableBott();
    wprintf(L"\n");
    wchar_t ptxt[64];
    swprintf(ptxt, 64, L"Страница %d [из %d]", p, total);
    PrintCentered(ptxt, WIDE);
    wprintf(L"\n\n");
    return lst;
}

List *NextPage(List *lst) {
    if (!lst) return lst;
    List *t = lst;
    for (int i = 0; i < 10 && t; i++) t = t->next;
    return t ? t : lst;
}

List *PrevPage(List *lst) {
    if (!lst) return NULL;
    List *t = lst;
    for (int i = 0; i < 10 && t->prev; i++) t = t->prev;
    return t;
}

Scientist ReadScientist(void) {
    Scientist a;
    a.id = -1;
    GetStr(a.fam, 32, L"Фамилию", 1);
    GetStr(a.im, 32, L"Имя", 1);
    GetStr(a.ot, 32, L"Отчество", 1);
    GetStr(a.obl, 32, L"Область", 1);
    GetStr(a.step, 32, L"Ученую степень", 1);
    a.stat = GetInt(L"количество статей");
    a.hirsh = GetInt(L"индекс Хирша");
    a.cit = GetInt(L"количество цитирований");
    return a;
}

void PrintScientist(Scientist a) {
    wprintf(L"ID: %d | %ls %ls %ls | Область: %ls | Степень: %ls | Статей: %d | Хирш: %d | Цит: %d\n",
            a.id, a.fam, a.im, a.ot, a.obl, a.step, a.stat, a.hirsh, a.cit);
}

List *FindById(List *lst, int id) {
    for (List *t = lst; t; t = t->next) if (t->data.id == id) return t;
    return NULL;
}

List *AddFirst(List *lst, Scientist a) {
    List *t = (List*)malloc(sizeof(List));
    t->data = a;
    t->next = lst;
    t->prev = NULL;
    if (lst) lst->prev = t;
    return t;
}

List *AddLast(List *lst, Scientist a) {
    List *t = (List*)malloc(sizeof(List));
    t->data = a;
    t->next = NULL;
    if (!lst) { t->prev = NULL; return t; }
    List *end = lst;
    while (end->next) end = end->next;
    end->next = t;
    t->prev = end;
    return lst;
}

void FreeList(List *lst) {
    while (lst) {
        List *tmp = lst;
        lst = lst->next;
        free(tmp);
    }
}

int NextId(List *lst) {
    if (!lst) return 1;
    int max = 0;
    for (List *t = lst; t; t = t->next) if (t->data.id > max) max = t->data.id;
    return max + 1;
}

int PickField(void) {
    int sel = 0;
    const char *items[] = {"Фамилия","Научная область","Ученая степень","Количество статей","Индекс Хирша","Количество цитирований"};
    while (1) {
        DrawMenu(sel, items, 6, L"Выберите поле для сортировки");
        int key = GetKey();
        if (key == KEY_UP) sel = (sel - 1 < 0) ? 5 : sel - 1;
        else if (key == KEY_DOWN) sel = (sel + 1 >= 6) ? 0 : sel + 1;
        else if (key == KEY_ESC) return -1;
        else if (key == KEY_ENTER) return sel;
    }
}

void QuickSortLow(List *left, List *right, int (*cmp)(const Scientist*, const Scientist*)) {
    if (!right || left == right || left == right->next) return;
    List *pivot = right;
    List *i = left->prev;
    for (List *j = left; j != right; j = j->next) {
        if (cmp(&j->data, &pivot->data) < 0) {
            i = (i == NULL) ? left : i->next;
            SwapSci(&i->data, &j->data);
        }
    }
    i = (i == NULL) ? left : i->next;
    SwapSci(&i->data, &pivot->data);
    QuickSortLow(left, i->prev, cmp);
    QuickSortLow(i->next, right, cmp);
}

void QuickSortHigh(List *left, List *right, int (*cmp)(const Scientist*, const Scientist*)) {
    if (!right || left == right || left == right->next) return;
    List *pivot = right;
    List *i = left->prev;
    for (List *j = left; j != right; j = j->next) {
        if (cmp(&j->data, &pivot->data) > 0) {
            i = (i == NULL) ? left : i->next;
            SwapSci(&i->data, &j->data);
        }
    }
    i = (i == NULL) ? left : i->next;
    SwapSci(&i->data, &pivot->data);
    QuickSortHigh(left, i->prev, cmp);
    QuickSortHigh(i->next, right, cmp);
}

List *SortLow(List *lst, int (*cmp)(const Scientist*, const Scientist*)) {
    if (!lst || !lst->next) return lst;
    List *left = lst; while (left->prev) left = left->prev;
    List *right = lst; while (right->next) right = right->next;
    QuickSortLow(left, right, cmp);
    return lst;
}

List *SortHigh(List *lst, int (*cmp)(const Scientist*, const Scientist*)) {
    if (!lst || !lst->next) return lst;
    List *left = lst; while (left->prev) left = left->prev;
    List *right = lst; while (right->next) right = right->next;
    QuickSortHigh(left, right, cmp);
    return lst;
}

int CmpName(const Scientist *a, const Scientist *b) { return wcscmp(a->fam, b->fam); }
int CmpField(const Scientist *a, const Scientist *b) { return wcscmp(a->obl, b->obl); }
int CmpDegree(const Scientist *a, const Scientist *b) { return wcscmp(a->step, b->step); }
int CmpArticles(const Scientist *a, const Scientist *b) { return a->stat - b->stat; }
int CmpHIndex(const Scientist *a, const Scientist *b) { return a->hirsh - b->hirsh; }
int CmpCitations(const Scientist *a, const Scientist *b) { return a->cit - b->cit; }
void SwapSci(Scientist *a, Scientist *b) { Scientist tmp = *a; *a = *b; *b = tmp; }

void ViewSearchResults(List *results, const wchar_t *title) {
    if (!results) {
        PrintTop(W);
        PrintCenter(L"Нет результатов для отображения", W);
        PrintBottom(W);
        ExitScreen();
        return;
    }
    int page = 1, key;
    List *cur = results;
    while (cur->prev) cur = cur->prev;
    int total = TotalPages(results);
    while (1) {
        printf("\033[2J\033[H");
        wprintf(L"\n");
        PrintTop(WIDE);
        PrintCenter(title, WIDE);
        PrintBottom(WIDE);
        wprintf(L"\n");
        PrintPage(cur, page, total);
        BlueBtn(L"Выход", WIDE);
        wprintf(L"\n");
        PrintTop(WIDE);
        PrintHelp(L"Стрелка влево  - Предыдущая страница", WIDE);
        PrintHelp(L"Стрелка вправо - Следующая страница", WIDE);
        PrintHelp(L"Enter / ESC    - Выйти", WIDE);
        PrintBottom(WIDE);
        key = GetKey();
        if (key == KEY_RIGHT) {
            List *tmp = NextPage(cur);
            if (tmp != cur) { cur = tmp; page++; }
        } else if (key == KEY_LEFT) {
            List *tmp = PrevPage(cur);
            if (tmp != cur) { cur = tmp; if (page > 1) page--; }
        } else if (key == KEY_ENTER || key == KEY_ESC) {
            return;
        }
    }
}

void ShowResult(List *lst, int type, const wchar_t *strVal, int num) {
    List *results = NULL;
    for (List *t = lst; t; t = t->next) {
        int ok = 0;
        switch (type) {
            case 0: if (CmpStr(strVal, t->data.fam) == 0) ok = 1; break;
            case 1: if (CmpStr(strVal, t->data.obl) == 0) ok = 1; break;
            case 2: if (CmpStr(strVal, t->data.step) == 0) ok = 1; break;
            case 3: if (t->data.stat == num) ok = 1; break;
            case 4: if (t->data.hirsh == num) ok = 1; break;
            case 5: if (t->data.cit == num) ok = 1; break;
        }
        if (ok) results = AddLast(results, t->data);
    }
    if (!results) {
        PrintTop(W);
        PrintCenter(L"Совпадений не найдено", W);
        PrintBottom(W);
        ExitScreen();
        return;
    }
    wchar_t title[200];
    if (type < 3) swprintf(title, 200, L"Результаты поиска: %ls", strVal);
    else swprintf(title, 200, L"Результаты поиска: значение %d", num);
    ViewSearchResults(results, title);
    FreeList(results);
}

void ShowRange(List *lst, int type, int lo, int hi) {
    List *results = NULL;
    for (List *t = lst; t; t = t->next) {
        int ok = 0;
        if (type == 3 && t->data.stat >= lo && t->data.stat <= hi) ok = 1;
        else if (type == 4 && t->data.hirsh >= lo && t->data.hirsh <= hi) ok = 1;
        else if (type == 5 && t->data.cit >= lo && t->data.cit <= hi) ok = 1;
        if (ok) results = AddLast(results, t->data);
    }
    if (!results) {
        PrintTop(W);
        PrintCenter(L"Совпадений не найдено", W);
        PrintBottom(W);
        ExitScreen();
        return;
    }
    wchar_t title[200];
    if (type == 3) swprintf(title, 200, L"Результаты поиска: статьи от %d до %d", lo, hi);
    else if (type == 4) swprintf(title, 200, L"Результаты поиска: индекс Хирша от %d до %d", lo, hi);
    else swprintf(title, 200, L"Результаты поиска: цитирования от %d до %d", lo, hi);
    ViewSearchResults(results, title);
    FreeList(results);
}

void CollectUnique(List *lst, int type, wchar_t vals[][32], int *cnt) {
    *cnt = 0;
    for (List *t = lst; t; t = t->next) {
        wchar_t *val = (type == 1) ? t->data.obl : t->data.step;
        int exists = 0;
        for (int i = 0; i < *cnt; i++) {
            if (CmpStr(val, vals[i]) == 0) { exists = 1; break; }
        }
        if (!exists && *cnt < 100) {
            wcscpy(vals[*cnt], val);
            (*cnt)++;
        }
    }
}

int PickValue(wchar_t vals[][32], int cnt, wchar_t *out) {
    if (cnt == 0) { ErrorBox(L"Нет доступных значений для выбора"); return 0; }
    int sel = 0;
    while (1) {
        printf("\033[2J\033[H");
        PrintTop(W);
        PrintCenter(L"Выберите значение из списка", W);
        PrintBottom(W);
        wprintf(L"\n");
        PrintTop(W);
        for (int i = 0; i < cnt; i++) {
            if (i == sel) wprintf(L"│  \033[97;44m > %-42ls < \033[0m  │\n", vals[i]);
            else wprintf(L"│     %-42ls     │\n", vals[i]);
        }
        PrintBottom(W);
        wprintf(L"\n");
        PrintTop(W);
        PrintHelp(L"Стрелки Вверх/Вниз - Листать", W);
        PrintHelp(L"Enter - Выбрать", W);
        PrintHelp(L"ESC   - Отмена", W);
        PrintBottom(W);
        int key = GetKey();
        if (key == KEY_UP) sel = (sel - 1 < 0) ? cnt - 1 : sel - 1;
        else if (key == KEY_DOWN) sel = (sel + 1 >= cnt) ? 0 : sel + 1;
        else if (key == KEY_ESC) return 0;
        else if (key == KEY_ENTER) { wcscpy(out, vals[sel]); return 1; }
    }
}

void FindName(List *lst) {
    wchar_t val[100];
    GetStr(val, 100, L"Фамилию для поиска", 0);
    ShowResult(lst, 0, val, 0);
}

void FindField(List *lst) {
    wchar_t uniq[100][32];
    int cnt;
    CollectUnique(lst, 1, uniq, &cnt);
    if (cnt == 0) { ErrorBox(L"Нет данных для поиска по области"); GetKey(); return; }
    wchar_t out[32];
    if (!PickValue(uniq, cnt, out)) return;
    ShowResult(lst, 1, out, 0);
}

void FindDegree(List *lst) {
    wchar_t uniq[100][32];
    int cnt;
    CollectUnique(lst, 2, uniq, &cnt);
    if (cnt == 0) { ErrorBox(L"Нет данных для поиска по ученой степени"); GetKey(); return; }
    wchar_t out[32];
    if (!PickValue(uniq, cnt, out)) return;
    ShowResult(lst, 2, out, 0);
}

void FindArticles(List *lst) {
    int lo, hi;
    if (!GetRange(L"Поиск по количеству статей в диапазоне:", &lo, &hi)) { GetKey(); return; }
    ShowRange(lst, 3, lo, hi);
}

void FindHIndex(List *lst) {
    int lo, hi;
    if (!GetRange(L"Поиск по индексу Хирша в диапазоне:", &lo, &hi)) { GetKey(); return; }
    ShowRange(lst, 4, lo, hi);
}

void FindCitations(List *lst) {
    int lo, hi;
    if (!GetRange(L"Поиск по количеству цитирований в диапазоне:", &lo, &hi)) { GetKey(); return; }
    ShowRange(lst, 5, lo, hi);
}

void ExitScreen(void) {
    wprintf(L"\n");
    BlueBtn(L"Выход", W);
    wprintf(L"\n");
    PrintTop(W);
    PrintHelp(L"Enter / ESC    - Выйти в главное меню", W);
    PrintBottom(W);
    GetKey();
}

int SaveTxt(List *lst) {
    char name[100];
    PrintTop(W);
    PrintCenter(L"Сохранение в текстовый файл", W);
    PrintBottom(W);
    wprintf(L"Имя файла для сохранения: ");
    GetFileName(name, 100);
    FILE *f = fopen(name, "w");
    if (!f) { ShowErrorAndExit(L"Не удалось открыть файл для записи"); return 1; }
    for (List *t = lst; t; t = t->next) {
        fwprintf(f, L"%d\n", t->data.id);
        fwprintf(f, L"%ls\n", t->data.fam);
        fwprintf(f, L"%ls\n", t->data.im);
        fwprintf(f, L"%ls\n", t->data.ot);
        fwprintf(f, L"%ls\n", t->data.obl);
        fwprintf(f, L"%ls\n", t->data.step);
        fwprintf(f, L"%d\n", t->data.stat);
        fwprintf(f, L"%d\n", t->data.hirsh);
        fwprintf(f, L"%d\n", t->data.cit);
    }
    fclose(f);
    wprintf(L"\n");
    PrintTop(W);
    PrintCenter(L"Таблица успешно сохранена", W);
    PrintBottom(W);
    ExitScreen();
    return 0;
}

int SaveBin(List *lst) {
    char name[100];
    PrintTop(W);
    PrintCenter(L"Сохранение в бинарный файл", W);
    PrintBottom(W);
    wprintf(L"Имя бинарного файла: ");
    GetFileName(name, 100);
    FILE *f = fopen(name, "wb");
    if (!f) { ShowErrorAndExit(L"Не удалось открыть бинарный файл для записи"); return 1; }
    for (List *t = lst; t; t = t->next) fwrite(&t->data, SZ, 1, f);
    fclose(f);
    wprintf(L"\n");
    PrintTop(W);
    PrintCenter(L"Бинарный файл успешно сохранен", W);
    PrintBottom(W);
    ExitScreen();
    return 0;
}

int CheckTxtFile(FILE *f) {
    wchar_t line[256];
    for (int i = 0; i < 9; i++) {
        if (!fgetws(line, 256, f)) return 0;
        line[wcslen(line)-1] = L'\0';
        if (i == 0) {
            int id;
            if (swscanf(line, L"%d", &id) != 1) return 0;
            if (id < 1 || id > 1000000) return 0;
        } else {
            if (wcslen(line) == 0) return 0;
        }
    }
    rewind(f);
    return 1;
}

List *LoadTxt(List *lst) {
    char name[100];
    PrintTop(W);
    PrintCenter(L"Чтение из текстового файла", W);
    PrintBottom(W);
    wprintf(L"Имя файла для загрузки: ");
    GetFileName(name, 100);
    FILE *f = fopen(name, "r");
    if (!f) { ShowErrorAndExit(L"Не удалось открыть файл для чтения"); return lst; }
    if (!CheckTxtFile(f)) { fclose(f); ShowErrorAndExit(L"Файл не соответствует формату"); return lst; }
    Scientist a;
    wchar_t line[256];
    while (1) {
        if (!fgetws(line, 256, f)) break;
        line[wcslen(line)-1] = L'\0';
        if (swscanf(line, L"%d", &a.id) != 1) break;
        if (!fgetws(a.fam, 32, f)) break; a.fam[wcslen(a.fam)-1] = L'\0';
        if (!fgetws(a.im, 32, f)) break; a.im[wcslen(a.im)-1] = L'\0';
        if (!fgetws(a.ot, 32, f)) break; a.ot[wcslen(a.ot)-1] = L'\0';
        if (!fgetws(a.obl, 32, f)) break; a.obl[wcslen(a.obl)-1] = L'\0';
        if (!fgetws(a.step, 32, f)) break; a.step[wcslen(a.step)-1] = L'\0';
        if (!fgetws(line, 256, f)) break; if (swscanf(line, L"%d", &a.stat) != 1) break;
        if (!fgetws(line, 256, f)) break; if (swscanf(line, L"%d", &a.hirsh) != 1) break;
        if (!fgetws(line, 256, f)) break; if (swscanf(line, L"%d", &a.cit) != 1) break;
        lst = AddLast(lst, a);
    }
    fclose(f);
    wprintf(L"\n");
    PrintTop(W);
    PrintCenter(L"Данные успешно загружены", W);
    PrintBottom(W);
    ExitScreen();
    return lst;
}

int CheckBinFile(FILE *f) {
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);
    if (size == 0 || size % SZ != 0) return 0;
    Scientist a;
    if (fread(&a, SZ, 1, f) != 1) return 0;
    rewind(f);
    if (a.id < 1 || a.id > 1000000) return 0;
    if (wcslen(a.fam) == 0) return 0;
    return 1;
}

List *LoadBin(List *lst) {
    char name[100];
    PrintTop(W);
    PrintCenter(L"Чтение из бинарного файла", W);
    PrintBottom(W);
    wprintf(L"Имя бинарного файла для загрузки: ");
    GetFileName(name, 100);
    FILE *f = fopen(name, "rb");
    if (!f) { ShowErrorAndExit(L"Не удалось открыть бинарный файл для чтения"); return lst; }
    if (!CheckBinFile(f)) { fclose(f); ShowErrorAndExit(L"Файл не соответствует формату"); return lst; }
    Scientist a;
    while (fread(&a, SZ, 1, f) == 1) lst = AddLast(lst, a);
    fclose(f);
    wprintf(L"\n");
    PrintTop(W);
    PrintCenter(L"Бинарные данные успешно загружены", W);
    PrintBottom(W);
    ExitScreen();
    return lst;
}

void ShowErrorAndExit(const wchar_t *msg) {
    printf("\033[2J\033[H");
    wprintf(L"\n");
    PrintTop(W);
    PrintCenter(L"ОШИБКА", W);
    PrintCenter(msg, W);
    PrintBottom(W);
    wprintf(L"\n");
    BlueBtn(L"Выход", W);
    wprintf(L"\n");
    PrintTop(W);
    PrintHelp(L"Enter / ESC    - Выйти в главное меню", W);
    PrintBottom(W);
    GetKey();
}

List *CreateAdd(List *lst) {
    while (1) {
        printf("\033[2J\033[H");
        PrintTop(W);
        PrintCenter(L"Добавление записи", W);
        PrintBottom(W);
        wprintf(L"\n");
        Scientist a = ReadScientist();
        a.id = NextId(lst);
        lst = (!lst) ? AddFirst(lst, a) : AddLast(lst, a);
        
        int sub = 0;
        const char *subItems[] = {"Выполнить", "Сохранить таблицу в файл", "Выход"};
        while (1) {
            DrawMenu(sub, subItems, 3, L"Запись добавлена. Хотите сделать еще одну запись?");
            int key = GetKey();
            if (key == KEY_UP) sub = (sub - 1 < 0) ? 2 : sub - 1;
            else if (key == KEY_DOWN) sub = (sub + 1 >= 3) ? 0 : sub + 1;
            else if (key == KEY_ESC) return lst;
            else if (key == KEY_ENTER) {
                if (sub == 0) break;
                else if (sub == 1) { SaveMenu(lst); return lst; }
                else if (sub == 2) return lst;
            }
        }
    }
}

List *CreateTable(List *lst) {
    int sel = 0;
    const char *yesno[] = {"Да", "Нет"};
    while (1) {
        DrawMenu(sel, yesno, 2, L"Создать новую таблицу? Данные будут очищены");
        int key = GetKey();
        if (key == KEY_UP) sel = (sel - 1 < 0) ? 1 : sel - 1;
        else if (key == KEY_DOWN) sel = (sel + 1 >= 2) ? 0 : sel + 1;
        else if (key == KEY_ESC || (key == KEY_ENTER && sel == 1)) return lst;
        else if (key == KEY_ENTER && sel == 0) break;
    }
    if (lst) {
        FreeList(lst);
        lst = NULL;
    }
    int src_sel = 0;
    const char *src_items[] = {"Загрузить из текстового файла", "Загрузить из бинарного файла", "Пропустить загрузку", "Выход"};
    while (1) {
        DrawMenu(src_sel, src_items, 4, L"Выберите источник данных для новой таблицы");
        int key = GetKey();
        if (key == KEY_UP) src_sel = (src_sel - 1 < 0) ? 3 : src_sel - 1;
        else if (key == KEY_DOWN) src_sel = (src_sel + 1 >= 4) ? 0 : src_sel + 1;
        else if (key == KEY_ESC) return lst;
        else if (key == KEY_ENTER) {
            if (src_sel == 0) {
                printf("\033[2J\033[H");  // очищаем экран перед загрузкой
                lst = LoadTxt(lst);
                break;
            } else if (src_sel == 1) {
                printf("\033[2J\033[H");  // очищаем экран перед загрузкой
                lst = LoadBin(lst);
                break;
            } else if (src_sel == 2) {
                break;  // пропустить загрузку
            } else if (src_sel == 3) {
                return lst;  // выход
            }
        }
    }
    lst = CreateAdd(lst);
    return lst;
}

void ViewTable(List *lst) {
    if (!lst) {
        PrintTop(W);
        PrintCenter(L"Таблица пуста", W);
        PrintBottom(W);
        ExitScreen();
        return;
    }
    int page = 1, key;
    List *cur = lst;
    while (1) {
        int total = TotalPages(lst);
        printf("\033[2J\033[H");
        wprintf(L"\n");
        PrintTop(WIDE);
        PrintCenter(L"База данных об ученых", WIDE);
        PrintBottom(WIDE);
        wprintf(L"\n");
        PrintPage(cur, page, total);
        BlueBtn(L"Выход", WIDE);
        wprintf(L"\n");
        PrintTop(WIDE);
        PrintHelp(L"Стрелка влево  - Предыдущая страница", WIDE);
        PrintHelp(L"Стрелка вправо - Следующая страница", WIDE);
        PrintHelp(L"Enter / ESC    - Выйти в главное меню", WIDE);
        PrintBottom(WIDE);
        key = GetKey();
        if (key == KEY_RIGHT) {
            List *tmp = NextPage(cur);
            if (tmp != cur) { cur = tmp; page++; }
        } else if (key == KEY_LEFT) {
            List *tmp = PrevPage(cur);
            if (tmp != cur) { cur = tmp; if (page > 1) page--; }
        } else if (key == KEY_ENTER || key == KEY_ESC) return;
    }
}

List *DeleteById(List *lst) {
    if (!lst) {
        PrintTop(W);
        PrintCenter(L"Таблица пуста", W);
        PrintBottom(W);
        ExitScreen();
        return lst;
    }
    PrintTop(W);
    PrintCenter(L"Удаление записи по ID", W);
    PrintBottom(W);
    int id;
    while (1) {
        wprintf(L"Введите ID для удаления (0 - выход): ");
        wchar_t buf[50];
        if (!fgetws(buf, 50, stdin)) continue;
        if (swscanf(buf, L"%d", &id) != 1) { ErrorBox(L"Введите числовое значение"); continue; }
        if (id == 0) return lst;
        break;
    }
    List *t = FindById(lst, id);
    if (!t) { wprintf(L"Запись не найдена\n"); ExitScreen(); return lst; }
    if (!t->prev && !t->next) lst = NULL;
    else if (!t->prev) { lst = lst->next; lst->prev = NULL; }
    else if (!t->next) { t->prev->next = NULL; }
    else { t->prev->next = t->next; t->next->prev = t->prev; }
    free(t);
    wprintf(L"\n");
    PrintTop(W);
    PrintCenter(L"Запись успешно удалена", W);
    PrintBottom(W);
    ExitScreen();
    return lst;
}

void EditRecord(List *lst) {
    if (!lst) {
        PrintTop(W);
        PrintCenter(L"Таблица пуста", W);
        PrintBottom(W);
        ExitScreen();
        return;
    }
    int id;
    PrintTop(W);
    PrintCenter(L"Редактирование записи", W);
    PrintBottom(W);
    while (1) {
        wprintf(L"Введите ID ученого для редактирования (0 для выхода): ");
        wchar_t buf[50];
        if (!fgetws(buf, 50, stdin)) continue;
        if (swscanf(buf, L"%d", &id) != 1) { ErrorBox(L"Введите корректное числовое ID"); continue; }
        if (id == 0) return;
        List *t = FindById(lst, id);
        if (!t) { ErrorBox(L"Ученый с таким ID не найден"); continue; }
        int sel = 0;
        const int szm = 9;
        const char *items[] = {"Фамилия","Имя","Отчество","Научная область","Ученая степень",
                               "Количество статей","Индекс Хирша","Количество цитирований","Все поля"};
        while (1) {
            DrawMenu(sel, items, szm, L"Что вы хотите изменить?");
            int key = GetKey();
            if (key == KEY_UP) { sel = (sel - 1 < 0) ? szm - 1 : sel - 1; }
            else if (key == KEY_DOWN) { sel = (sel + 1 >= szm) ? 0 : sel + 1; }
            else if (key == KEY_ESC) return;
            else if (key == KEY_ENTER) {
                printf("\033[2J\033[H");
                PrintTop(W);
                PrintCenter(L"Ввод новых данных", W);
                PrintBottom(W);
                wprintf(L"\n");
                int curId = t->data.id, dummy;
                switch (sel) {
                    case 0: GetStr(t->data.fam,32,L"Фамилию",1); break;
                    case 1: GetStr(t->data.im,32,L"Имя",1); break;
                    case 2: GetStr(t->data.ot,32,L"Отчество",1); break;
                    case 3: GetStr(t->data.obl,32,L"Область",1); break;
                    case 4: GetStr(t->data.step,32,L"Ученую степень",1); break;
                    case 5: t->data.stat = GetInt(L"количество статей"); break;
                    case 6: t->data.hirsh = GetInt(L"индекс Хирша"); break;
                    case 7: t->data.cit = GetInt(L"количество цитирований"); break;
                    case 8: t->data = ReadScientist(); t->data.id = curId; break;
                }
                break;
            }
        }
        break;
    }
    wprintf(L"\n");
    PrintTop(W);
    PrintCenter(L"Изменения сохранены", W);
    PrintBottom(W);
    ExitScreen();
}

List *SortTable(List *lst) {
    if (!lst) {
        PrintTop(W);
        PrintCenter(L"Таблица пуста", W);
        PrintBottom(W);
        ExitScreen();
        return lst;
    }
    int f = PickField();
    if (f == -1) return lst;
    int (*cmp)(const Scientist*, const Scientist*) = NULL;
    switch (f) {
        case 0: cmp = CmpName; break;
        case 1: cmp = CmpField; break;
        case 2: cmp = CmpDegree; break;
        case 3: cmp = CmpArticles; break;
        case 4: cmp = CmpHIndex; break;
        case 5: cmp = CmpCitations; break;
    }
    int dir = 0;
    const char *dirs[] = {"По возрастанию","По убыванию","Выход"};
    while (1) {
        DrawMenu(dir, dirs, 3, L"Направление сортировки");
        int key = GetKey();
        if (key == KEY_UP) { dir = (dir - 1 < 0) ? 2 : dir - 1; }
        else if (key == KEY_DOWN) { dir = (dir + 1 >= 3) ? 0 : dir + 1; }
        else if (key == KEY_ESC) return lst;
        else if (key == KEY_ENTER) {
            if (dir == 0) lst = SortLow(lst, cmp);
            if (dir == 1) lst = SortHigh(lst, cmp);
            break;
        }
    }
    printf("\033[2J\033[H");
    PrintTop(W);
    PrintCenter(L"Результат операции", W);
    PrintBottom(W);
    wprintf(L"\n");
    PrintTop(W);
    PrintCenter(L"Сортировка успешно завершена", W);
    PrintBottom(W);
    ExitScreen();
    return lst;
}

void SearchRecord(List *lst) {
    if (!lst) {
        PrintTop(W);
        PrintCenter(L"Таблица пуста", W);
        PrintBottom(W);
        ExitScreen();
        return;
    }
    int sel = 0;
    const char *items[] = {"Фамилия","Научная область","Ученая степень","Кол-во статей","Индекс Хирша","Кол-во цитирований","Выход"};
    while (1) {
        DrawMenu(sel, items, 7, L"Поиск записи по критериям");
        int key = GetKey();
        if (key == KEY_UP) { sel = (sel - 1 < 0) ? 6 : sel - 1; }
        else if (key == KEY_DOWN) { sel = (sel + 1 >= 7) ? 0 : sel + 1; }
        else if (key == KEY_ESC) return;
        else if (key == KEY_ENTER) {
            printf("\033[2J\033[H");
            if (sel == 6) return;
            PrintTop(W);
            PrintCenter(L"Параметры поиска", W);
            PrintBottom(W);
            wprintf(L"\n");
            switch (sel) {
                case 0: FindName(lst); return;
                case 1: FindField(lst); return;
                case 2: FindDegree(lst); return;
                case 3: FindArticles(lst); return;
                case 4: FindHIndex(lst); return;
                case 5: FindCitations(lst); return;
            }
        }
    }
}

void SaveMenu(List *lst) {
    if (!lst) {
        PrintTop(W);
        PrintCenter(L"Сохранять нечего", W);
        PrintBottom(W);
        ExitScreen();
        return;
    }
    int sel = 0;
    const char *m[] = {"В текстовый файл","В бинарный файл","Выход"};
    while (1) {
        DrawMenu(sel, m, 3, L"Экспорт данных");
        int key = GetKey();
        if (key == KEY_UP) { sel = (sel - 1 < 0) ? 2 : sel - 1; }
        else if (key == KEY_DOWN) { sel = (sel + 1 >= 3) ? 0 : sel + 1; }
        else if (key == KEY_ESC) return;
        else if (key == KEY_ENTER) {
            printf("\033[2J\033[H");
            if (sel == 0) SaveTxt(lst);
            if (sel == 1) SaveBin(lst);
            return;
        }
    }
}

List *LoadMenu(List *lst) {
    if (lst) {
        int sel = 0;
        const char *m[] = {"Перезаписать таблицу","Дополнить таблицу","Выход"};
        while (1) {
            DrawMenu(sel, m, 3, L"Таблица содержит данные. Что сделать?");
            int key = GetKey();
            if (key == KEY_UP) { sel = (sel - 1 < 0) ? 2 : sel - 1; }
            else if (key == KEY_DOWN) { sel = (sel + 1 >= 3) ? 0 : sel + 1; }
            else if (key == KEY_ESC) return lst;
            else if (key == KEY_ENTER) break;
        }
        if (sel == 2) return lst;
        if (sel == 0) { FreeList(lst); lst = NULL; }
    }
    int sel = 0;
    const char *m[] = {"Из текстового файла","Из бинарного файла","Выход"};
    while (1) {
        DrawMenu(sel, m, 3, L"Импорт данных");
        int key = GetKey();
        if (key == KEY_UP) { sel = (sel - 1 < 0) ? 2 : sel - 1; }
        else if (key == KEY_DOWN) { sel = (sel + 1 >= 3) ? 0 : sel + 1; }
        else if (key == KEY_ESC) return lst;
        else if (key == KEY_ENTER) break;
    }
    printf("\033[2J\033[H");
    if (sel == 0) lst = LoadTxt(lst);
    if (sel == 1) lst = LoadBin(lst);
    return lst;
}

void TopFive(List *lst) {
    if (!lst) {
        PrintTop(W);
        PrintCenter(L"Таблица пуста", W);
        PrintBottom(W);
        ExitScreen();
        return;
    }
    wchar_t fields[100][32];
    int fcnt = 0;
    for (List *t = lst; t; t = t->next) {
        int exists = 0;
        for (int i = 0; i < fcnt; i++) {
            if (CmpStr(t->data.obl, fields[i]) == 0) { exists = 1; break; }
        }
        if (!exists && fcnt < 100) wcscpy(fields[fcnt++], t->data.obl);
    }
    int cur = 0, key, need = 1;
    while (1) {
        if (need) {
            printf("\033[2J\033[H");
            wprintf(L"\n");
            PrintTop(WIDE);
            PrintCenter(L"Результаты обработки данных по областям", WIDE);
            wchar_t title[120];
            swprintf(title, 120, L"Научная область: %ls", fields[cur]);
            PrintCenter(title, WIDE);
            PrintBottom(WIDE);
            wprintf(L"\n");
            int cnt = 0;
            for (List *t = lst; t; t = t->next) if (CmpStr(t->data.obl, fields[cur]) == 0) cnt++;
            Scientist *arr = (Scientist*)malloc(cnt * sizeof(Scientist));
            int idx = 0;
            for (List *t = lst; t; t = t->next) if (CmpStr(t->data.obl, fields[cur]) == 0) arr[idx++] = t->data;
            wchar_t fio[100], obl[32], step[32];
            // сортировка по Хиршу (убывание)
            for (int i = 0; i < cnt - 1; i++) {
                for (int j = i + 1; j < cnt; j++) {
                    if (arr[i].hirsh < arr[j].hirsh) { Scientist tmp = arr[i]; arr[i] = arr[j]; arr[j] = tmp; }
                }
            }
            wprintf(L"┌───────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┐\n");
            PrintCenter(L"Топ 5 ученых с наибольшим индексом Хирша", WIDE);
            wprintf(L"├────┬────────────────────────────────────────┬─────────────────┬────────────────────┬────────────┬──────────┬──────────┤\n");
            wprintf(L"│ ID │ ФИО                                    │ Область         │ Учёная степень     │  Статей    │  Хирш    │  Цитир   │\n");
            wprintf(L"├────┼────────────────────────────────────────┼─────────────────┼────────────────────┼────────────┼──────────┼──────────┤\n");
            int lim = (cnt < 5) ? cnt : 5;
            for (int i = 0; i < lim; i++) {
                swprintf(fio, 100, L"%ls %ls %ls", arr[i].fam, arr[i].im, arr[i].ot);
                wcscpy(obl, arr[i].obl);
                wcscpy(step, arr[i].step);
                ShortenString(fio, 38);
                ShortenString(obl, 15);
                ShortenString(step, 18);
                wprintf(L"│%3d │ %-38ls │ %-15ls │ %-18ls │ %10d │ %8d │ %8d │\n",
                        arr[i].id, fio, obl, step, arr[i].stat, arr[i].hirsh, arr[i].cit);
                if (i < lim - 1) PrintDivider();
            }
            PrintTableBott();
            wprintf(L"\n");
            idx = 0;
            for (List *t = lst; t; t = t->next) if (CmpStr(t->data.obl, fields[cur]) == 0) arr[idx++] = t->data;
            for (int i = 0; i < cnt - 1; i++) {
                for (int j = i + 1; j < cnt; j++) {
                    if (arr[i].cit < arr[j].cit) { Scientist tmp = arr[i]; arr[i] = arr[j]; arr[j] = tmp; }
                }
            }
            wprintf(L"┌───────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┐\n");
            PrintCenter(L"Топ 5 ученых с наибольшим числом цитирований", WIDE);
            wprintf(L"├────┬────────────────────────────────────────┬─────────────────┬────────────────────┬────────────┬──────────┬──────────┤\n");
            wprintf(L"│ ID │ ФИО                                    │ Область         │ Учёная степень     │  Статей    │  Хирш    │  Цитир   │\n");
            wprintf(L"├────┼────────────────────────────────────────┼─────────────────┼────────────────────┼────────────┼──────────┼──────────┤\n");
            for (int i = 0; i < lim; i++) {
                swprintf(fio, 100, L"%ls %ls %ls", arr[i].fam, arr[i].im, arr[i].ot);
                wcscpy(obl, arr[i].obl);
                wcscpy(step, arr[i].step);
                ShortenString(fio, 38);
                ShortenString(obl, 15);
                ShortenString(step, 18);
                wprintf(L"│%3d │ %-38ls │ %-15ls │ %-18ls │ %10d │ %8d │ %8d │\n",
                        arr[i].id, fio, obl, step, arr[i].stat, arr[i].hirsh, arr[i].cit);
                if (i < lim - 1) PrintDivider();
            }
            PrintTableBott();
            wprintf(L"\n");
            free(arr);
            wchar_t ptext[64];
            swprintf(ptext, 64, L"Страница %d из %d", cur + 1, fcnt);
            PrintCentered(ptext, WIDE);
            wprintf(L"\n\n");
            BlueBtn(L"Выход", WIDE);
            wprintf(L"\n");
            PrintTop(WIDE);
            PrintHelp(L"Стрелка влево  - Предыдущая страница", WIDE);
            PrintHelp(L"Стрелка вправо - Следующая страница", WIDE);
            PrintHelp(L"Enter / ESC    - Выйти в главное меню", WIDE);
            PrintBottom(WIDE);
            need = 0;
        }
        key = GetKey();
        if (key == KEY_RIGHT && cur < fcnt - 1) { cur++; need = 1; }
        else if (key == KEY_LEFT && cur > 0) { cur--; need = 1; }
        else if (key == KEY_ENTER || key == KEY_ESC) return;
    }
}

void ExitProg(List *lst) {
    FreeList(lst);
    printf("\033[2J\033[H");
}

int main() {
    setlocale(LC_ALL, "");
    int sel = 0, key;
    List *lst = NULL;
    const int cnt = 11;
    const char *items[] = {
        "Создать таблицу",
        "Добавить запись",
        "Просмотреть таблицу",
        "Удалить запись из таблицы",
        "Редактировать запись",
        "Сортировать таблицу",
        "Поиск записи в таблице",
        "Сохранить таблицу в файл",
        "Загрузить таблицу из файла",
        "Вывести топ 5 ученых",
        "Выход из программы"
    };
    while (1) {
        DrawMenu(sel, items, cnt, L"Главное меню базы данных об ученых");
        key = GetKey();
        if (key == KEY_UP) { sel = (sel - 1 < 0) ? cnt - 1 : sel - 1; }
        else if (key == KEY_DOWN) { sel = (sel + 1 >= cnt) ? 0 : sel + 1; }
        else if (key == KEY_ESC) { ExitProg(lst); return 0; }
        else if (key == KEY_ENTER) {
            printf("\033[2J\033[H");
            switch (sel) {
                case 0: lst = CreateTable(lst); break;
                case 1: lst = CreateAdd(lst); break;
                case 2: ViewTable(lst); break;
                case 3: lst = DeleteById(lst); break;
                case 4: EditRecord(lst); break;
                case 5: lst = SortTable(lst); break;
                case 6: SearchRecord(lst); break;
                case 7: SaveMenu(lst); break;
                case 8: lst = LoadMenu(lst); break;
                case 9: TopFive(lst); break;
                case 10: ExitProg(lst); return 0;
            }
        }
    }
    return 0;
}