undefined4 main(void)
{
    char cVar1;
    int32_t iVar2;
    uint32_t uVar3;
    uint8_t *puVar4;
    char *pcVar5;
    uint8_t *puVar6;
    bool bVar7;
    undefined uVar8;
    undefined uVar9;
    bool bVar10;
    undefined uVar11;
    uint8_t uVar12;
    uint8_t auStack144 [5];
    char acStack139 [2];
    undefined auStack137 [125];
    int32_t var_bp_8h;
    
    uVar12 = 0;
    do {
        printf("%p, %p \n", _auth, _service);
        iVar2 = fgets(auStack144, 0x80, _reloc.stdin);
        bVar7 = false;
        bVar10 = iVar2 == 0;
        if (bVar10) {
            return 0;
        }
        iVar2 = 5;
        puVar4 = auStack144;
        puVar6 = (uint8_t *)"auth ";
        do {
            if (iVar2 == 0) break;
            iVar2 = iVar2 + -1;
            bVar7 = *puVar4 < *puVar6;
            bVar10 = *puVar4 == *puVar6;
            puVar4 = puVar4 + (uint32_t)uVar12 * -2 + 1;
            puVar6 = puVar6 + (uint32_t)uVar12 * -2 + 1;
        } while (bVar10);
        uVar8 = 0;
        uVar11 = (!bVar7 && !bVar10) == bVar7;
        if ((bool)uVar11) {
            _auth = (undefined4 *)malloc(4);
            *_auth = 0;
            uVar3 = 0xffffffff;
            pcVar5 = acStack139;
            do {
                if (uVar3 == 0) break;
                uVar3 = uVar3 - 1;
                cVar1 = *pcVar5;
                pcVar5 = pcVar5 + (uint32_t)uVar12 * -2 + 1;
            } while (cVar1 != '\0');
            uVar3 = ~uVar3 - 1;
            uVar8 = uVar3 < 0x1e;
            uVar11 = uVar3 == 0x1e;
            if (uVar3 < 0x1f) {
                strcpy(_auth, acStack139);
            }
        }
        iVar2 = 5;
        puVar4 = auStack144;
        puVar6 = (uint8_t *)"reset";
        do {
            if (iVar2 == 0) break;
            iVar2 = iVar2 + -1;
            uVar8 = *puVar4 < *puVar6;
            uVar11 = *puVar4 == *puVar6;
            puVar4 = puVar4 + (uint32_t)uVar12 * -2 + 1;
            puVar6 = puVar6 + (uint32_t)uVar12 * -2 + 1;
        } while ((bool)uVar11);
        uVar9 = 0;
        uVar8 = (!(bool)uVar8 && !(bool)uVar11) == (bool)uVar8;
        if ((bool)uVar8) {
            free(_auth);
        }
        iVar2 = 6;
        puVar4 = auStack144;
        puVar6 = (uint8_t *)"service";
        do {
            if (iVar2 == 0) break;
            iVar2 = iVar2 + -1;
            uVar9 = *puVar4 < *puVar6;
            uVar8 = *puVar4 == *puVar6;
            puVar4 = puVar4 + (uint32_t)uVar12 * -2 + 1;
            puVar6 = puVar6 + (uint32_t)uVar12 * -2 + 1;
        } while ((bool)uVar8);
        uVar11 = 0;
        uVar8 = (!(bool)uVar9 && !(bool)uVar8) == (bool)uVar9;
        if ((bool)uVar8) {
            uVar11 = (uint8_t *)0xfffffff8 < auStack144;
            uVar8 = auStack137 == (undefined *)0x0;
            _service = strdup(auStack137);
        }
        iVar2 = 5;
        puVar4 = auStack144;
        puVar6 = (uint8_t *)"login";
        do {
            if (iVar2 == 0) break;
            iVar2 = iVar2 + -1;
            uVar11 = *puVar4 < *puVar6;
            uVar8 = *puVar4 == *puVar6;
            puVar4 = puVar4 + (uint32_t)uVar12 * -2 + 1;
            puVar6 = puVar6 + (uint32_t)uVar12 * -2 + 1;
        } while ((bool)uVar8);
        if ((!(bool)uVar11 && !(bool)uVar8) == (bool)uVar11) {
            if (_auth[8] == 0) {
                fwrite("Password:\n", 1, 10, _reloc.stdout);
            } else {
                system("/bin/sh");
            }
        }
    } while( true );
}
