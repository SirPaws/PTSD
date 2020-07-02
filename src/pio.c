#include "pio.h"
#include <Windows.h>
#include <math.h>

#include "allocator.h"
extern Allocator *pCurrentAllocatorFunc;
extern void *pCurrentAllocatorUserData;

enum StreamType {
    STANDARD_STREAM,
    FILE_STREAM,
    STRING_STREAM
};

struct GenericStream {
    enum StreamType  type;
    u32 flags;
    void *buffer;
};

struct StringStream {
    const enum StreamType  type;
    u32 flags;
    usize buffersize;
    usize actualsize;
    u8 *c_str;
    // maybe more if not we just expose stringstream
};

struct FileStream {
    const enum StreamType  type;
    const u32 flags;
#if defined(_WIN32) || defined(_WIN64)
    HANDLE filehandle;
#endif
};

struct StdStream {
    const enum StreamType  type;
    const enum StreamFlags flags;
#if defined(_WIN32) || defined(_WIN64)
    HANDLE stdout_handle;
    HANDLE stdin_handle;
#endif
};

void InitializeStdStream(void) __attribute__(( constructor ));
void DestroyStdStream(void) __attribute__(( destructor ));

static StdStream *StandardStream = NULL;
static u32 default_code_page = 0;
void InitializeStdStream(void) __attribute__(( constructor ));
void DestroyStdStream(void) __attribute__(( destructor ));

static GenericStream *pcurrentstream;

GenericStream *pSetStream(GenericStream *stream) {
    assert(stream);
    GenericStream *output = pcurrentstream;
    pcurrentstream = stream;
    return output;
}
GenericStream *pGetStream(void) { return pcurrentstream; }

void InitializeStdStream(void) {
#if defined(_WIN32) || defined(_WIN64)
    default_code_page = GetConsoleOutputCP();
    SetConsoleOutputCP(CP_UTF8);
#endif
    const StdStream template = { 
        .type          = STANDARD_STREAM,
        .flags         = STREAM_INPUT|STREAM_OUTPUT,
        .stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE),
        .stdin_handle  = GetStdHandle(STD_INPUT_HANDLE)
    };

    StandardStream = malloc(sizeof *StandardStream);
    pcurrentstream = (void *)StandardStream;
    memcpy(StandardStream, &template, sizeof template);

}
void DestroyStdStream(void) {
    free(StandardStream);
#if defined(_WIN32) || defined(_WIN64)
    SetConsoleOutputCP(default_code_page);
#endif
}


StdStream *pGetStandardStream(enum StreamFlags flags) {
    union {
        const enum StreamFlags *flags;
        int *mutable_flags;
    } conv;

    StdStream *std = pCurrentAllocatorFunc(NULL, sizeof *std, 0, MALLOC, pCurrentAllocatorUserData);
    memcpy(std, StandardStream, sizeof *std);
    conv.flags = &std->flags; *conv.mutable_flags = flags; 

    if ((flags & STREAM_INPUT) == 0)
        std->stdin_handle = NULL;
    if ((flags & STREAM_OUTPUT) == 0)
        std->stdout_handle = NULL;
    return std;
}

#define expect(x, value) __builtin_expect(x, value)

void StreamWriteString(GenericStream *stream, String str) {
    assert(stream->flags & STREAM_OUTPUT);

    if (expect(stream->type == STANDARD_STREAM, 1) || stream->type == FILE_STREAM) {
        FileStream *fstream = (FileStream *)stream;
        WriteFile(fstream->filehandle, str.c_str, (u32)str.length, NULL, NULL);
    } else {
        // type == STRING_STREAM
        StringStream *sstream = (StringStream *)stream;
        usize diff = (sstream->buffersize - sstream->actualsize);
        if (diff <= str.length) {
            void *tmp = pCurrentAllocatorFunc(sstream->c_str, 
                    sstream->buffersize + str.length, 0, REALLOC, pCurrentAllocatorUserData);
            sstream->c_str = tmp; 
            sstream->buffersize += str.length;
        }
        memcpy(sstream->c_str + sstream->actualsize, str.c_str, str.length);
        sstream->actualsize += str.length;
    }
}

void StreamWriteChar(GenericStream *stream, char str) {
    assert(stream->flags & STREAM_OUTPUT);

    if (expect(stream->type == STANDARD_STREAM, 1) || stream->type == FILE_STREAM) {
        FileStream *fstream = (FileStream *)stream;
        WriteFile(fstream->filehandle, &str, 1, NULL, NULL);
    } else {
        // type == STRING_STREAM
        StringStream *sstream = (StringStream *)stream;
        usize diff = (sstream->buffersize - sstream->actualsize);
        if (diff <= 1) {
            void *tmp = pCurrentAllocatorFunc(sstream->c_str, 
                    sstream->buffersize + 1, 0, REALLOC, pCurrentAllocatorUserData);
            sstream->c_str = tmp; 
            sstream->buffersize += 1;
        }
        sstream->c_str[sstream->actualsize] = (u8)str;
        sstream->actualsize += 1;
    }
}

static bool IsCharacters(int character, u32 count, char tests[count]){
    for (u32 i = 0; i < count; i++) {
        if (character == tests[i]) return true;
    }
    return false;
}

u64 PrintJustified(GenericStream *stream, String string, bool right_justified, const u8 character, u32 count);

void GetRGB(const char *restrict* fmt, String RGB[3]);
u32  GetUnicodeLength(const char *chr);

struct JusticifationInfo {
    bool right_justified;
    u32 justification_count;
    u8 justificiation_chars[2];
    bool justification_char_is_zero;
};

struct BinaryStringReturn {
    u8 *buffer;
    String str;
    bool iszero;
};

struct BinaryStringReturn MakeBinaryString(u64 bitcount, u64 num);
struct BinaryStringReturn MakeBinaryString(u64 bitcount, u64 num) {
    struct BinaryStringReturn ret = { 0 };

    u64 bit = 1ULL << (bitcount - 1);
    ret.buffer = pCurrentAllocatorFunc(NULL, bitcount, 0, MALLOC, pCurrentAllocatorUserData);
    for (u64 i = 0; i < bitcount; i++) {
       ret.buffer[i] = (u8)'0' + ((num & bit) ? 1 : 0); 
       bit >>= 1;
    }

    ret.str.c_str = ret.buffer;
    s32 count = (s32)bitcount;
    while(*ret.str.c_str == '0') { ret.str.c_str++; count--; }
    if (count <= 0) { 
        ret.iszero = true;
        ret.str.c_str -= 1; 
        count = 1; 
    }
    ret.str.length = (usize)count; 
    return ret;
}

u32 pVBPrintf(GenericStream *stream, const char *restrict fmt, va_list list) {
    u32 printcount = 0;

    while(*fmt) {
        if (expect(*fmt != '%', 1)) {
            const char *restrict fmt_next = fmt;
            while (IsCharacters(*fmt_next, 2, (char[2]){ '%', '\0'}) == false) fmt_next++;
            StreamWrite(pcurrentstream, (String){ (const u8 *)fmt, (usize)(fmt_next - fmt)});
            fmt = fmt_next;
        } else {
            bool always_print_sign  = false; 
            u32 bitcount = 32;
            bool bitcountset = false;
            struct JusticifationInfo info = {
                .right_justified    = false,
                .justification_count = 0,
                .justificiation_chars = { u' ', u'0' },
                .justification_char_is_zero  = false,
            };
            const char *restrict fmt_next = fmt + 1;
        repeat:
            #define SetBitCount(n, increment) bitcount = n; bitcountset = true; fmt_next += increment
            switch(*fmt_next) {
                case '-': info.right_justified             = true; fmt_next++; goto repeat; 
                case '+': always_print_sign                = true; fmt_next++; goto repeat;
                case '0': info.justification_char_is_zero  = true; fmt_next++; goto repeat;

                case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                    {
                        const char *restrict begin = fmt_next;
                        char *end;
                        info.justification_count = strtoul(begin, &end, 10);
                        // maybe fmt_next = ++end; not sure tbh
                        fmt_next = end; 
                        goto repeat;
                    }
                case 'h': {
                        if (*(fmt_next + 1) == 'h') { 
                            bitcount = sizeof(char) * 8;
                            fmt_next += 2;
                        }
                        else {
                            bitcount = sizeof(short)  * 8;
                            fmt_next++;
                        }
                        goto repeat;
                    }
                case 'l': {
                        if (*(fmt_next + 1) == 'l'){ 
                            bitcount = sizeof(long long) * 8;
                            fmt_next += 2;
                        }
                        else {
                            bitcount = sizeof(long)  * 8;
                            fmt_next++;
                        }
                        goto repeat;
                    }
                case 'j': SetBitCount(sizeof(intmax_t)    * 8, 1); goto repeat;
                case 'z': SetBitCount(sizeof(size_t)      * 8, 1); goto repeat;
                case 't': SetBitCount(sizeof(ptrdiff_t)   * 8, 1); goto repeat; 
                case 'L': SetBitCount(sizeof(long double) * 8, 1); goto repeat;
                
                case 's': case 'S': {
                        const u8 character = info.justificiation_chars[info.justification_char_is_zero];
                        String str;
                        if (*fmt_next == 's'){
                            const char *c_str = va_arg(list, const char *);
                            str = (String){ .c_str = (const u8 *)c_str, .length = strlen(c_str) };
                        } else { 
                            str = va_arg(list, String); 
                        }
                        PrintJustified(stream, str, info.right_justified, 
                                character, info.justification_count);
                        break;
                    }
                case 'i':case 'd': {
                        const u8 character = info.justificiation_chars[info.justification_char_is_zero];
                        u32 count;
                        char buf[20];
                        s64 num = 0;
                        if (expect(bitcount <= 32, 1))
                             num = va_arg(list, s32);
                        else num = va_arg(list, s64);
                        count = pItoa(buf, num);
                        char *printbuf = buf;
                        if (num > 0 && !always_print_sign) { printbuf++; count--; }
                        PrintJustified(stream, (String){ (u8 *)printbuf, count }, 
                                info.right_justified, character, info.justification_count);
                    } break;
                case 'u': {
                        const u8 character = info.justificiation_chars[info.justification_char_is_zero];
                        u32 count;
                        char buf[20];
                        u64 num = 0; 
                        if (expect(bitcount <= 32, 1))
                             num = va_arg(list, u32);
                        else num = va_arg(list, u64);
                        count = pUtoa(buf, num);
                        PrintJustified(stream, (String){ (u8 *)buf, count }, 
                                info.right_justified, character, info.justification_count);
                    } break;
                case 'b': {
                        const u8 character = info.justificiation_chars[info.justification_char_is_zero];
                        u64 num = va_arg(list, u64);
                        struct BinaryStringReturn ret = MakeBinaryString(bitcount > 64 ? 64 : bitcount, num);
                        u32 old_justification_count = info.justification_count;
                        if (info.justification_count == 0 && info.justification_char_is_zero) {
                            info.justification_count = bitcount; 
                        }
                        PrintJustified(stream, ret.str, info.right_justified, 
                                character, info.justification_count);
                        info.justification_count = old_justification_count;
                        pCurrentAllocatorFunc(ret.buffer, 0, 0, FREE, pCurrentAllocatorUserData);
                        break;
                    }
                case 'c': {
                    if (expect(bitcountset == false, 1)){
                        int character = va_arg(list, int);
                        StreamWrite(stream, (char)character);
                    }
                    else {
                        const char *character = va_arg(list, const char *);
                        u32 len = GetUnicodeLength(character);
                        StreamWrite(stream, (String){ (const u8 *)character, len});
                    }
                    break;
                }
                case 'C': {
                        if (memcmp(fmt_next, "Cc", 2) == 0) {
                            StreamWrite(stream, pCreateString("\x1b[0m"));
                            fmt_next++;
                            break;
                        }
                        else if (memcmp(fmt_next, "CB", 2) == 0) {
                            FormatCallBack *callback = va_arg(list, FormatCallBack *);
                             
                            fmt_next += 2;
                            FormatCallbackTuple ret = callback(stream, fmt_next, list);
                            fmt_next = ret.end_pos - 1;
                            list = ret.list;
                            break;
                        }
                        else if (memcmp(fmt_next, "Cbg", 3) == 0) {
                            fmt_next += 3;
                            if (*fmt_next != '(') break;
                            StreamWrite(stream, pCreateString("\x1b[48;2;"));
                        }
                        else if (memcmp(fmt_next, "Cfg", 3) == 0) {
                            fmt_next += 3;
                            if (*fmt_next != '(') break;
                            StreamWrite(stream, pCreateString("\x1b[38;2;"));
                        }
                        String RGB[3];
                        GetRGB(&fmt_next, RGB);
                        StreamWrite(stream, RGB[0]);
                        StreamWrite(stream, ';');
                        StreamWrite(stream, RGB[1]);
                        StreamWrite(stream, ';');
                        StreamWrite(stream, RGB[2]);
                        StreamWrite(stream, pCreateString("m"));
                    }
                    break;
                default: break;
            }
            fmt = ++fmt_next;
        }
    }
    return printcount;
}
u32 pBPrintf(GenericStream *stream, const char *restrict fmt, ...);

u64 PrintJustified(GenericStream *stream, String string, bool right_justified, const u8 character, u32 count) {
    s64 test = (s64)count - (s64)string.length;
    if (test > 0) {
        char *arr = pCurrentAllocatorFunc(NULL, (u64)test, 0, MALLOC, pCurrentAllocatorUserData);
        void *tmp = memset(arr, character, (u64)test); arr = tmp;

        if (expect(!right_justified, 1)) {
            StreamWrite(stream, (String){ tmp, (u64)test });
            StreamWrite(stream, string);
        } else {
            StreamWrite(stream, string);
            StreamWrite(stream, (String){ tmp, (u64)test });
        }
        pCurrentAllocatorFunc(arr, 0, 0, FREE, pCurrentAllocatorUserData);
        return string.length + (u64)test;
    } else {
        StreamWrite(stream, string);
        return string.length;
    }
}

void GetRGB(const char *restrict* fmtptr, String RGB[3]) {
    const char *restrict fmt = *fmtptr;
    int n = 0;
    while(*fmt != ')') {
        if (n >= 3) break;
        const u8 *begin = (const u8*)fmt + 1;
        const u8 *end   = (const u8*)fmt + 1;
        while(IsCharacters(*end, 2, (char[2]){ ',', ')'}) == false) end++;
        RGB[n++] = (String){ begin, (usize)(end - begin) }; 
        fmt = (const char *)end;
    }
    *fmtptr = fmt;
}

u32 GetUnicodeLength(const char *chr) {
    /**/ if ((chr[0] & 0xf8) == 0xf0)
         return 4;
    else if ((chr[0] & 0xf0) == 0xe0)
         return 3;
    else if ((chr[0] & 0xe0) == 0xc0)
         return 2;
    else return 1;
}

static const char *Itoa100[100] = {
    "00", "10", "20", "30", "40", "50", "60", "70", "80", "90",
    "01", "11", "21", "31", "41", "51", "61", "71", "81", "91",
    "02", "12", "22", "32", "42", "52", "62", "72", "82", "92",
    "03", "13", "23", "33", "43", "53", "63", "73", "83", "93",
    "04", "14", "24", "34", "44", "54", "64", "74", "84", "94",
    "05", "15", "25", "35", "45", "55", "65", "75", "85", "95",
    "06", "16", "26", "36", "46", "56", "66", "76", "86", "96",
    "07", "17", "27", "37", "47", "57", "67", "77", "87", "97",
    "08", "18", "28", "38", "48", "58", "68", "78", "88", "98",
    "09", "19", "29", "39", "49", "59", "69", "79", "89", "99",
};

static const char *Itoa1000[1000] = {
    "000", "100", "200", "300", "400", "500", "600", "700", "800", "900",
    "010", "110", "210", "310", "410", "510", "610", "710", "810", "910",
    "020", "120", "220", "320", "420", "520", "620", "720", "820", "920",
    "030", "130", "230", "330", "430", "530", "630", "730", "830", "930",
    "040", "140", "240", "340", "440", "540", "640", "740", "840", "940",
    "050", "150", "250", "350", "450", "550", "650", "750", "850", "950",
    "060", "160", "260", "360", "460", "560", "660", "760", "860", "960",
    "070", "170", "270", "370", "470", "570", "670", "770", "870", "970",
    "080", "180", "280", "380", "480", "580", "680", "780", "880", "980",
    "090", "190", "290", "390", "490", "590", "690", "790", "890", "990",

    "001", "101", "201", "301", "401", "501", "601", "701", "801", "901",
    "011", "111", "211", "311", "411", "511", "611", "711", "811", "911",
    "021", "121", "221", "321", "421", "521", "621", "721", "821", "921",
    "031", "131", "231", "331", "431", "531", "631", "731", "831", "931",
    "041", "141", "241", "341", "441", "541", "641", "741", "841", "941",
    "051", "151", "251", "351", "451", "551", "651", "751", "851", "951",
    "061", "161", "261", "361", "461", "561", "661", "761", "861", "961",
    "071", "171", "271", "371", "471", "571", "671", "771", "871", "971",
    "081", "181", "281", "381", "481", "581", "681", "781", "881", "981",
    "091", "191", "291", "391", "491", "591", "691", "791", "891", "991",

    "002", "102", "202", "302", "402", "502", "602", "702", "802", "902",
    "012", "112", "212", "312", "412", "512", "612", "712", "812", "912",
    "022", "122", "222", "322", "422", "522", "622", "722", "822", "922",
    "032", "132", "232", "332", "432", "532", "632", "732", "832", "932",
    "042", "142", "242", "342", "442", "542", "642", "742", "842", "942",
    "052", "152", "252", "352", "452", "552", "652", "752", "852", "952",
    "062", "162", "262", "362", "462", "562", "662", "762", "862", "962",
    "072", "172", "272", "372", "472", "572", "672", "772", "872", "972",
    "082", "182", "282", "382", "482", "582", "682", "782", "882", "982",
    "092", "192", "292", "392", "492", "592", "692", "792", "892", "992",

    "003", "103", "203", "303", "403", "503", "603", "703", "803", "903",
    "013", "113", "213", "313", "413", "513", "613", "713", "813", "913",
    "023", "123", "223", "323", "423", "523", "623", "723", "823", "923",
    "033", "133", "233", "333", "433", "533", "633", "733", "833", "933",
    "043", "143", "243", "343", "443", "543", "643", "743", "843", "943",
    "053", "153", "253", "353", "453", "553", "653", "753", "853", "953",
    "063", "163", "263", "363", "463", "563", "663", "763", "863", "963",
    "073", "173", "273", "373", "473", "573", "673", "773", "873", "973",
    "083", "183", "283", "383", "483", "583", "683", "783", "883", "983",
    "093", "193", "293", "393", "493", "593", "693", "793", "893", "993",

    "004", "104", "204", "304", "404", "504", "604", "704", "804", "904",
    "014", "114", "214", "314", "414", "514", "614", "714", "814", "914",
    "024", "124", "224", "324", "424", "524", "624", "724", "824", "924",
    "034", "134", "234", "334", "434", "534", "634", "734", "834", "934",
    "044", "144", "244", "344", "444", "544", "644", "744", "844", "944",
    "054", "154", "254", "354", "454", "554", "654", "754", "854", "954",
    "064", "164", "264", "364", "464", "564", "664", "764", "864", "964",
    "074", "174", "274", "374", "474", "574", "674", "774", "874", "974",
    "084", "184", "284", "384", "484", "584", "684", "784", "884", "984",
    "094", "194", "294", "394", "494", "594", "694", "794", "894", "994",

    "005", "105", "205", "305", "405", "505", "605", "705", "805", "905",
    "015", "115", "215", "315", "415", "515", "615", "715", "815", "915",
    "025", "125", "225", "325", "425", "525", "625", "725", "825", "925",
    "035", "135", "235", "335", "435", "535", "635", "735", "835", "935",
    "045", "145", "245", "345", "445", "545", "645", "745", "845", "945",
    "055", "155", "255", "355", "455", "555", "655", "755", "855", "955",
    "065", "165", "265", "365", "465", "565", "665", "765", "865", "965",
    "075", "175", "275", "375", "475", "575", "675", "775", "875", "975",
    "085", "185", "285", "385", "485", "585", "685", "785", "885", "985",
    "095", "195", "295", "395", "495", "595", "695", "795", "895", "995",

    "006", "106", "206", "306", "406", "506", "606", "706", "806", "906",
    "016", "116", "216", "316", "416", "516", "616", "716", "816", "916",
    "026", "126", "226", "326", "426", "526", "626", "726", "826", "926",
    "036", "136", "236", "336", "436", "536", "636", "736", "836", "936",
    "046", "146", "246", "346", "446", "546", "646", "746", "846", "946",
    "056", "156", "256", "356", "456", "556", "656", "756", "856", "956",
    "066", "166", "266", "366", "466", "566", "666", "766", "866", "966",
    "076", "176", "276", "376", "476", "576", "676", "776", "876", "976",
    "086", "186", "286", "386", "486", "586", "686", "786", "886", "986",
    "096", "196", "296", "396", "496", "596", "696", "796", "896", "996",

    "007", "107", "207", "307", "407", "507", "607", "707", "807", "907",
    "017", "117", "217", "317", "417", "517", "617", "717", "817", "917",
    "027", "127", "227", "327", "427", "527", "627", "727", "827", "927",
    "037", "137", "237", "337", "437", "537", "637", "737", "837", "937",
    "047", "147", "247", "347", "447", "547", "647", "747", "847", "947",
    "057", "157", "257", "357", "457", "557", "657", "757", "857", "957",
    "067", "167", "267", "367", "467", "567", "667", "767", "867", "967",
    "077", "177", "277", "377", "477", "577", "677", "777", "877", "977",
    "087", "187", "287", "387", "487", "587", "687", "787", "887", "987",
    "097", "197", "297", "397", "497", "597", "697", "797", "897", "997",

    "008", "108", "208", "308", "408", "508", "608", "708", "808", "908",
    "018", "118", "218", "318", "418", "518", "618", "718", "818", "918",
    "028", "128", "228", "328", "428", "528", "628", "728", "828", "928",
    "038", "138", "238", "338", "438", "538", "638", "738", "838", "938",
    "048", "148", "248", "348", "448", "548", "648", "748", "848", "948",
    "058", "158", "258", "358", "458", "558", "658", "758", "858", "958",
    "068", "168", "268", "368", "468", "568", "668", "768", "868", "968",
    "078", "178", "278", "378", "478", "578", "678", "778", "878", "978",
    "088", "188", "288", "388", "488", "588", "688", "788", "888", "988",
    "098", "198", "298", "398", "498", "598", "698", "798", "898", "998",

    "009", "109", "209", "309", "409", "509", "609", "709", "809", "909",
    "019", "119", "219", "319", "419", "519", "619", "719", "819", "919",
    "029", "129", "229", "329", "429", "529", "629", "729", "829", "929",
    "039", "139", "239", "339", "439", "539", "639", "739", "839", "939",
    "049", "149", "249", "349", "449", "549", "649", "749", "849", "949",
    "059", "159", "259", "359", "459", "559", "659", "759", "859", "959",
    "069", "169", "269", "369", "469", "569", "669", "769", "869", "969",
    "079", "179", "279", "379", "479", "579", "679", "779", "879", "979",
    "089", "189", "289", "389", "489", "589", "689", "789", "889", "989",
    "099", "199", "299", "399", "499", "599", "699", "799", "899", "999",
};

u32 pItoa(char *buf, s64 num) {
    if (num < 0) {
        *buf++ = '-';
        num = llabs(num);
    } else *buf++ = '+';
    return pUtoa(buf, (u64)num) + 1; 
}

u32 pUtoa(char *buf, u64 num) {
    u32 printnum = 0;
    register char *ptr = buf;
    if (expect(num > 999, 1)) {
        while(num) {
            register u32 mod = num % 1000;
            num = num / 1000;
            if (num <= 0) {
                if (mod < 10){
                    *ptr++ = '0' + (char)mod;
                    printnum++;
                }
                else if (mod < 100) {
                    register const char *s = Itoa100[mod];
                    ptr[0] = s[0];
                    ptr[1] = s[1];
                    ptr += 2;
                    printnum += 2;
                }
            } else {
                register const char *s = Itoa1000[mod];
                ptr[0] = s[0];
                ptr[1] = s[1];
                ptr[2] = s[2];
                ptr += 3;
                printnum += 3;
            }
        }
    } else if (num < 10) {
        *ptr++ = '0' + (char)num;
        printnum++;
    } else if (num < 100) {
        register const char *s = Itoa100[num];
        ptr[0] = s[0];
        ptr[1] = s[1];
        ptr += 2;
        printnum += 2;
    }
    ptr--;
    while(ptr > buf) {
        const char tmp = *buf;
        *buf++ = *ptr;
        *ptr-- = tmp;
    }
    return printnum;
}

