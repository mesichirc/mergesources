typedef u32 ChatEventType;
enum {
  ChatEventNone = 0,
  ChatEventKeyPressed,
  ChatEventKeyReleased,
  ChatEventMouseBtnPressed,
  ChatEventMouseBtnReleased,
  ChatEventMouseScroll,
  ChatEventMousePositionChanged
};

typedef u32 ChatEventFlag;
enum {
  ChatKeyPressedFlag            = (1 << ChatEventKeyPressed),
  ChatKeyReleasedFlag           = (1 << ChatEventKeyReleased),
  ChatMouseScrollFlag           = (1 << ChatEventMouseScroll),
  ChatMouseBtnPressedFlag       = (1 << ChatEventMouseBtnPressed),
  ChatMouseBtnReleasedFlag      = (1 << ChatEventMouseBtnReleased),
  ChatMousePositionChangedFlag  = (1 << ChatEventMousePositionChanged)
};

typedef u32 ChatKeyCode;
enum {
  ChatKeyCodeNull = 0,
  ChatKeyCodeEscape = '\033',
  ChatKeyCodeBacktick = '`',
  ChatKeyCode0 = '0',
  ChatKeyCode1 = '1',
  ChatKeyCode2 = '2',
  ChatKeyCode3 = '3',
  ChatKeyCode4 = '4',
  ChatKeyCode5 = '5',
  ChatKeyCode6 = '6',
  ChatKeyCode7 = '7',
  ChatKeyCode8 = '8',
  ChatKeyCode9 = '9',
  ChatKeyCodeMinus = '-',
	ChatKeyCodeEquals = '=',
	ChatKeyCodeBackSpace = '\b',
	ChatKeyCodeTab = '\t',
	ChatKeyCodeSpace = ' ',
  ChatKeyCodeA = 'a',
	ChatKeyCodeB = 'b',
	ChatKeyCodeC = 'c',
	ChatKeyCodeD = 'd',
	ChatKeyCodeE = 'e',
	ChatKeyCodeF = 'f',
	ChatKeyCodeG = 'g',
	ChatKeyCodeH = 'h',
	ChatKeyCodeI = 'i',
	ChatKeyCodeJ = 'j',
	ChatKeyCodeK = 'k',
	ChatKeyCodeL = 'l',
	ChatKeyCodeM = 'm',
	ChatKeyCodeN = 'n',
	ChatKeyCodeO = 'o',
	ChatKeyCodeP = 'p',
	ChatKeyCodeQ = 'q',
	ChatKeyCodeR = 'r',
	ChatKeyCodeS = 's',
	ChatKeyCodeT = 't',
	ChatKeyCodeU = 'u',
	ChatKeyCodeV = 'v',
	ChatKeyCodeW = 'w',
	ChatKeyCodeX = 'x',
	ChatKeyCodeY = 'y',
	ChatKeyCodeZ = 'z',
  ChatKeyCodePeriod = '.',
	ChatKeyCodeComma = ',',
	ChatKeyCodeSlash = '/',
	ChatKeyCodeBracket = '[',
  ChatKeyCodeCloseBracket = ']',
  ChatKeyCodeSemicolon = ';',
	ChatKeyCodeApostrophe = '\'',
	ChatKeyCodeBackSlash = '\\',
	ChatKeyCodeReturn = '\n',
	ChatKeyCodeEnter = ChatKeyCodeReturn,
	ChatKeyCodeDelete = '\177', /* 127 */
	ChatKeyCodeF1,
	ChatKeyCodeF2,
	ChatKeyCodeF3,
	ChatKeyCodeF4,
	ChatKeyCodeF5,
	ChatKeyCodeF6,
	ChatKeyCodeF7,
	ChatKeyCodeF8,
	ChatKeyCodeF9,
	ChatKeyCodeF10,
	ChatKeyCodeF11,
	ChatKeyCodeF12,
  ChatKeyCodeF13,
  ChatKeyCodeF14,
  ChatKeyCodeF15,
  ChatKeyCodeF16,
  ChatKeyCodeF17,
  ChatKeyCodeF18,
  ChatKeyCodeF19,
  ChatKeyCodeF20,
  ChatKeyCodeF21,
  ChatKeyCodeF22,
  ChatKeyCodeF23,
  ChatKeyCodeF24,
  ChatKeyCodeF25,
	ChatKeyCodeCapsLock,
	ChatKeyCodeShiftL,
	ChatKeyCodeControlL,
	ChatKeyCodeAltL,
	ChatKeyCodeSuperL,
	ChatKeyCodeShiftR,
	ChatKeyCodeControlR,
	ChatKeyCodeAltR,
	ChatKeyCodeSuperR,
	ChatKeyCodeUp,
	ChatKeyCodeDown,
	ChatKeyCodeLeft,
	ChatKeyCodeRight,
	ChatKeyCodeInsert,
	ChatKeyCodeMenu,
	ChatKeyCodeEnd,
	ChatKeyCodeHome,
	ChatKeyCodePageUp,
	ChatKeyCodePageDown,
	ChatKeyCodeNumLock,
	ChatKeyCodeKpSlash,
	ChatKeyCodeKpMultiply,
	ChatKeyCodeKpPlus,
	ChatKeyCodeKpMinus,
	ChatKeyCodeKpEqual,
	ChatKeyCodeKp1,
	ChatKeyCodeKp2,
	ChatKeyCodeKp3,
	ChatKeyCodeKp4,
	ChatKeyCodeKp5,
	ChatKeyCodeKp6,
	ChatKeyCodeKp7,
	ChatKeyCodeKp8,
	ChatKeyCodeKp9,
	ChatKeyCodeKp0,
	ChatKeyCodeKpPeriod,
	ChatKeyCodeKpReturn,
	ChatKeyCodeScrollLock,
  ChatKeyCodePrintScreen,
  ChatKeyCodePause,
	ChatKeyCodeWorld1,
  ChatKeyCodeWorld2,
  ChatKeyCodeLast = 256
};

const char *keyCodesStrings[ChatKeyCodeLast];

void ChatInputKeyCodesDebugInit(void);

typedef struct ChatInput ChatInput;
struct ChatInput {
  u8            characters[512];
  u8            keyCodes[ChatKeyCodeLast];
  int           mouseX;
  int           mouseY;
  u16           charactersLength;
  ChatEventFlag flag;
  u8            mouseButtonsState[64]; // 0 left btn, 1 right button, other others buttons
  f32           deltaX;
  f32           deltaY;
};

typedef struct ChatInputHandle ChatInputHandle;
struct ChatInputHandle {
  ChatInput *newInput;
  ChatInput *prevInput;
};

void ChatInputPrint(ChatInput *input);

bool ChatInputKeyPressed(ChatInputHandle input, ChatKeyCode keyCode);


void ChatUpdateAndRender(Pepe_Bitmap *canvas, ChatInputHandle input, Pepe_Bitmap *defaultFont);
