void greetuser(int32_t arg_8h)
{
    char *s1;
    uint32_t uStack72;
    undefined4 uStack68;
    uint32_t uStack64;
    undefined2 uStack60;
    char cStack58;
    
    if (_language == 1) {
        s1 = "Hyvää päivää "._0_4_;
        uStack72 = "Hyvää päivää "._4_4_;
        uStack68 = "Hyvää päivää "._8_4_;
        uStack64 = "Hyvää päivää "._12_4_;
        uStack60 = "Hyvää päivää "._16_2_;
        cStack58 = "Hyvää päivää "[18];
    } else {
        if (_language == 2) {
            s1 = "Goedemiddag! "._0_4_;
            uStack72 = "Goedemiddag! "._4_4_;
            uStack68 = "Goedemiddag! "._8_4_;
            uStack64 = uStack64 & 0xffff0000 | (uint32_t)"Goedemiddag! "._12_2_;
        } else {
            if (_language == 0) {
                s1 = "Hello "._0_4_;
                uStack72._0_3_ = CONCAT12("Hello "[6], "Hello "._4_2_);
                uStack72 = uStack72 & 0xff000000 | (uint32_t)(unkuint3)uStack72;
            }
        }
    }
    strcat(&s1, &arg_8h);
    puts(&s1);
    return;
}
