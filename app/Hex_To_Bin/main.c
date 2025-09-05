#include "intel_hex.h"
#include <stdio.h>

int main(void)
{
    FILE *fp = fopen("application.hex", "r");
    if (!fp)
    {
        perror("Error opening file");
        return 1;
    }

    Intel_StatusTypeDef status = IntelHex_Read_AllLine(fp);
    fclose(fp);

    if (status != SUCCESS)
    {
        printf("Intel HEX file is invalid. Cannot convert.\n");
        return 1;
    }

    printf("Intel HEX file is valid. Converting to binary...\n");
    IntelHex_Convert_Hex_To_Bin("C:\\Users\\Adminstrator\\OneDrive\\Desktop\\stm32Fota\\app\\Hex_To_Bin\\application.hex", "C:\\Users\\Adminstrator\\OneDrive\\Desktop\\stm32Fota\\app\\Hex_To_Bin\\application.bin");
    return 0;
}
