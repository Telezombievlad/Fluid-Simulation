// DONE: Принимаем начальную картинку, симулируем, выдаём картинки.
// DONE: getPixel() вместо txGetPixel()
// DONE: картинка, лерп от цвета картинки к красному по RGB
// DONE: Стенки странной формы (всякие там препятствия)
// DONE: Массив теплопроводностей
// DONE: Первичен размер массива - размер окна константен - зумим.
//-----------------------------------------------------------------------------

#include "TXLib.h"
#include "mechanics/Classes.h"

//----------------------------------------------------------------------------
//{ Function prototypes
//----------------------------------------------------------------------------

    void saveScreenshot
    (
        const char* fileName,
        const unsigned int number,
        const unsigned int recommendedSize,
        const unsigned int width,
        const unsigned int height
    );

    bool txSaveBMP(const char* filename, HDC dc, int sizeX, int sizeY);

    void waitUntilSpaceButtonIsPressed();

    char* mergeStr(const char* str0, const char* str1);

    char* int2Str(unsigned int integer, const unsigned int recommendedArraySize);

//}
//-----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//{ [TOP SECRET] DEFINES FOR LAMBDA EXPRESSIONS
//----------------------------------------------------------------------------

    #define GRADIENT ([](const unsigned int x, const unsigned int y){return MAX_TEMPERATURE/2 - sqrt(pow((signed) ARRAY_WIDTH/2 - (signed) x, 2) + pow((signed) ARRAY_HEIGHT/2 - (signed) y, 2));})
    #define ELLIPSE  ([](const unsigned int x, const unsigned int y){return (20 * pow((signed) ARRAY_WIDTH/2 + 20 - (signed) x, 2) + pow((signed) ARRAY_HEIGHT/2 - (signed) y, 2) < 8100)? WARM_TEMPERATURE : COLD_TEMPERATURE;})

//}
//-----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//{ Constants
//----------------------------------------------------------------------------

    const unsigned int ARRAY_WIDTH = 300, ARRAY_HEIGHT = 300;

    const double COLD_TEMPERATURE = 100.0;
    const double WARM_TEMPERATURE = 10000.0;

    const unsigned int ZOOM = 3;

//}
//-----------------------------------------------------------------------------


int main()
{
    txCreateWindow(ARRAY_WIDTH * ZOOM, ARRAY_HEIGHT * ZOOM);
    txTextCursor(false);

    Field test = Field("resources/conductivity/hook.bmp", "resources/obstacles/hook.bmp", "resources/images/hook.bmp", ARRAY_WIDTH, ARRAY_HEIGHT, 0, 0, NULL);

    puts("[SIMULATION MODE]");

    for (unsigned int counter = 0, screenShotCounter = 0, screenShotNumber = 0; !GetAsyncKeyState(VK_ESCAPE); counter++)
    {
        // Conditions setting:
        if (GetAsyncKeyState(VK_RETURN)) test.editorMode(4, 100, ZOOM);

        test.adjustTemperature(105, 150, 7, 10);

        // Calculations:
        test.calculate();

        // Rendering:
        if (counter == 500)
        {
            counter = 0;

            screenShotCounter++;

            txSetFillColor(TX_BLACK);
            txClear();

            txBegin();

            test.render(ZOOM, GetAsyncKeyState('0'));

            txEnd();
        }

        // Saving screenshot:
        if (screenShotCounter == 5 && screenShotNumber < 20)
        {
            screenShotCounter = 0;

            saveScreenshot("resources/screenshots/hook/screen", screenShotNumber, 3, ARRAY_WIDTH * ZOOM, ARRAY_HEIGHT * ZOOM);

            screenShotNumber++;
        }
    }

    test.render(ZOOM, GetAsyncKeyState('0'));

    return 0;
}

//----------------------------------------------------------------------------
//{ Additional functions
//----------------------------------------------------------------------------

    // Screen saving:

    void saveScreenshot
    (
        const char* fileName,
        const unsigned int number,
        const unsigned int recommendedSize,
        const unsigned int width,
        const unsigned int height
    )
    {
        assert(fileName);

        txSaveBMP
        (
            mergeStr
            (
                mergeStr
                (
                    fileName,
                    int2Str
                    (
                        number,
                        log(number + 1)/log(10) + 1
                    )
                ),
                ".bmp"
            ),
            txDC(),
            width,
            height
        );
    }

    // Ded's txSaveBMP function
    bool txSaveBMP(const char* filename, HDC dc, int sizeX, int sizeY)
    {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wmultichar"

        assert (filename); assert (dc);

        FILE* f = fopen (filename, "wb");
        if (!f) return false;

        size_t szHdrs = sizeof (BITMAPFILEHEADER) + sizeof (BITMAPINFOHEADER),
               szImg  = (sizeX * sizeY) * sizeof (RGBQUAD);

        BITMAPFILEHEADER hdr  = { 'MB', szHdrs + szImg, 0, 0, szHdrs };
        BITMAPINFOHEADER info = { sizeof (info), sizeX, sizeY, 1, WORD (sizeof (RGBQUAD) * 8), BI_RGB };

        RGBQUAD* buf = new RGBQUAD [sizeX * sizeY]; assert (buf);

        Win32::GetDIBits (dc, (HBITMAP) Win32::GetCurrentObject (dc, OBJ_BITMAP),
                          0, sizeY, buf, (BITMAPINFO*) &info, DIB_RGB_COLORS);

        fwrite (&hdr,  sizeof (hdr),  1, f);
        fwrite (&info, sizeof (info), 1, f);
        fwrite (buf,   szImg,         1, f);

        delete[] buf;

        fclose (f);
        return true;

        #pragma GCC diagnostic pop
    }

    // Different string operations:

    char* mergeStr(const char* str0, const char* str1)
    {
        char* tmpStr = (char*) calloc(strlen(str0) + 1, sizeof(*tmpStr));
        assert(tmpStr);

        strcpy(tmpStr, str0);

        strcat(tmpStr, str1);

        return tmpStr;
    }

    char* int2Str(unsigned int integer, const unsigned int recommendedArraySize)
    {
        // Main algorithm:

            // We don't care about bulletproofness
            char* toReturn = (char*) calloc(recommendedArraySize + 1, sizeof(*toReturn));
            assert(toReturn);

            for (size_t index = recommendedArraySize - 1; 0 <= index && index < recommendedArraySize; index--)
            {
                assert(0 <= index && index < recommendedArraySize);

                toReturn[index] = '0' + integer % 10;

                integer /= 10;
            }

        // Checking output:

            assert(toReturn);

        // Return statement:

            return toReturn;
    }

//}
//----------------------------------------------------------------------------

