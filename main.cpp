#include "TXLib.h"
#include "mechanics/Classes.h"

void waitUntilSpaceButtonIsPressed();

#define WAIT waitUntilSpaceButtonIsPressed();

const unsigned int WIDTH = 600, HEIGHT = 600;
const double COLD_TEMPERATURE = 100.0;
const double WARM_TEMPERATURE = 10000.0;

int main()
{
    txCreateWindow(WIDTH, HEIGHT);
    txTextCursor(false);

    Field test = Field(WIDTH, HEIGHT, [](const unsigned int x, const unsigned int y)
                                      {return (pow((signed) WIDTH/2 - (signed) x, 2) + pow((signed) HEIGHT/2 - (signed) y, 2) < 10000 && x >= 200)? WARM_TEMPERATURE : COLD_TEMPERATURE;});

    // GRADIENT       - {return MAX_TEMPERATURE/2 - sqrt(pow((signed) WIDTH/2 - (signed) x, 2) + pow((signed) HEIGHT/2 - (signed) y, 2));}
    // ROUND          - {return (pow((signed) WIDTH/2 - (signed) x, 2) + pow((signed) HEIGHT/2 - (signed) y, 2) < 10000 && x >= 200)? WARM_TEMPERATURE : COLD_TEMPERATURE;}
    // HALF_ROUND     - {return (pow((signed) WIDTH/2 - (signed) x, 2) + pow((signed) HEIGHT/2 - (signed) y, 2) < 10000 && x >= 200)? WARM_TEMPERATURE : COLD_TEMPERATURE;}

    puts("[SIMULATION MODE]");

    for (unsigned int counter = 0; !GetAsyncKeyState(VK_ESCAPE); counter = (counter == 500)? 0 : counter + 1)
    {
        if (GetAsyncKeyState(VK_RETURN)) test.setFieldConditionsManually(25, 50);

        test.calculate();

        if (counter == 500)
        {
            txBegin();

            test.render();

            txEnd();
        }
    }

    test.render();

    return 0;
}

void waitUntilSpaceButtonIsPressed()
{
    bool spaceButtonPressed = (GetAsyncKeyState(VK_SPACE)) ? true : false;
    bool ready = !spaceButtonPressed;

    while (true)
    {
        if (!GetAsyncKeyState(VK_SPACE) && spaceButtonPressed) ready = true;

        if (ready && GetAsyncKeyState(VK_SPACE)) break;
    }
}
