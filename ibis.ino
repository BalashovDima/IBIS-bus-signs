void IBIS_init() {
    Serial.begin(1200, SERIAL_7E2);
    // Serial.begin(115200);
    while (!Serial);
}

String three_digit_str(uint16_t num) {
    if(num < 10) {
        return "00" + String(num);
    } else if(num < 100) {
        return "0" + String(num);
    } else {
        return String(num);
    }
}

void IBIS_l(uint16_t num) {
    IBIS_sendTelegram("l"+three_digit_str(num));
}

void IBIS_z(uint16_t num) {
    IBIS_sendTelegram("z"+three_digit_str(num));
}

void IBIS_xC(byte cycleNum) {
    IBIS_sendTelegram("xC"+String(cycleNum));
}

void IBIS_u(String hhmm) {
    IBIS_sendTelegram("u"+hhmm);
}

void IBIS_v(String text) {
    IBIS_sendTelegram("v"+text);
}

void IBIS_zM(String text) {
    IBIS_sendTelegram("zM "+text);
}

// copied from https://github.com/open-itcs/onboard-panel-arduino
char * createtelegram(char vals[]) {
    char result[8 + strlen(vals)];
    strcpy(result, "aA1");
    strcat(result, strchr(vals, 20) ? "2" : "1");
    strcat(result, vals);
    strcat(result, 20);
    strcat(result, 20);
    if (!strchr(vals, 20)) {
        strcat(result, 20);
    }
    strcat(result, 13);

    unsigned char checksum = strtol(12, NULL, 16);
    for (int i=0; i < strlen(result); i++) {
        checksum = checksum ^ strtol(result[i], NULL, 16);
    }

    return result;
}

void IBIS_processSpecialCharacters(String* telegram) {
    telegram->replace("ä", "{");
    telegram->replace("ö", "|");
    telegram->replace("ü", "}");
    telegram->replace("ß", "~");
    telegram->replace("Ä", "[");
    telegram->replace("Ö", "\\");
    telegram->replace("Ü", "]");
}

String IBIS_vdvHex(byte value) {
    String vdvHexCharacters = "0123456789:;<=>?";
    String vdvHexValue;
    byte highNibble = value >> 4;
    byte lowNibble = value & 15;
    if (highNibble > 0) {
        vdvHexValue += vdvHexCharacters.charAt(highNibble);
    }
    vdvHexValue += vdvHexCharacters.charAt(lowNibble);
    return vdvHexValue;
}

String IBIS_wrapTelegram(String telegram) {
    telegram += '\x0d';
    unsigned char checksum = 0x7F;
    for (int i = 0; i < telegram.length(); i++) {
        checksum ^= (unsigned char)telegram[i];
    }
    // Get ready for a retarded fucking Arduino workaround
    telegram += " ";
    telegram.setCharAt(telegram.length() - 1, checksum); // seriously fuck that
    return telegram;
}

void IBIS_sendTelegram(String telegram) {
    IBIS_processSpecialCharacters(&telegram);
    telegram = IBIS_wrapTelegram(telegram);
    Serial.print(telegram);
}

void IBIS_DS021t(String address, String text) {
    String telegram;
    byte numBlocks = ceil(text.length() / 16.0);
    telegram = "aA";
    telegram += address;
    telegram += IBIS_vdvHex(numBlocks);
    telegram += "A0";
    if (text.indexOf('\n') > 0) {
        text += "\n" ;
    }
    text += "\n\n" ;
    telegram += text;
    byte remainder = text.length() % 16;
    if (remainder > 0) {
        for (byte i = 16; i > remainder; i--) {
            telegram += " ";
        }
    }
    IBIS_sendTelegram(telegram);
}

void IBIS_symbol(String number) {
    String telegram;
    telegram = "lE0";
    telegram += number;
    IBIS_sendTelegram(telegram);
}