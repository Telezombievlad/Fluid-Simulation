#pragma once


//----------------------------------------------------------------------------
//{ Defines (typedefs)
//----------------------------------------------------------------------------

    typedef unsigned int size_t;

//}
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//{ Constants
//----------------------------------------------------------------------------

    // Physics:

        const double THERMAL_CONDUCTIVITY_COEFFICIENT = 1;

        const double MAX_TEMPERATURE = 2/*K*/;

        const double  TIME_STEP = 10;
        const double SPACE_STEP = 8;

    // Rendering:

        //COLORREF COLD_COLOR = RGB(  0, 0, 255);
        COLORREF  MID_COLOR = RGB(255, 110, 80);
        COLORREF WARM_COLOR = RGB(255, 245, 120);

        COLORREF WALL_COLOR = RGB(0, 0, 0);

    // Tile types:

        const unsigned char  EMPTY_TILE = 0,
                            BORDER_TILE = 1,
                              WALL_TILE = 2;

//}
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//{ Defines
//----------------------------------------------------------------------------

    #define WAIT waitUntilSpaceButtonIsPressed();

//}
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//{ Additional functions
//----------------------------------------------------------------------------

    // Waiting:

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

    // LERPs

    inline double lerp(const double a, const double b, const double k)
    {
        assert(0 <= k && k <= 1.0);

        return a + k * (b - a);
    }

    inline COLORREF lerp(const COLORREF a, const COLORREF b, const double k)
    {
        //printf("%f\n", k);

        assert(0 <= k && k <= 1.0);

        return RGB((char) lerp((double) GetRValue(a), (double) GetRValue(b), k),
                   (char) lerp((double) GetGValue(a), (double) GetGValue(b), k),
                   (char) lerp((double) GetBValue(a), (double) GetBValue(b), k));
    }

    COLORREF colorLerp(double temperature, COLORREF minColor, COLORREF midColor, COLORREF maxColor/*, const double lerpCoefficientDecrease*/)
    {
        // Preparing input:

            if (MAX_TEMPERATURE < temperature)
            {
                temperature = MAX_TEMPERATURE;
            }

            if (temperature < MAX_TEMPERATURE/2)
            {

                double lerpCoeffitient = temperature/(MAX_TEMPERATURE/2);
                /*
                if (lerpCoeffitient > lerpCoefficientDecrease) lerpCoeffitient = 0;
                else lerpCoeffitient -= lerpCoefficientDecrease;
                */
                return lerp(minColor, midColor,  lerpCoeffitient);
            }
            else
            {

                double lerpCoeffitient = (temperature - MAX_TEMPERATURE/2)/(MAX_TEMPERATURE/2);
                /*
                if (lerpCoeffitient > lerpCoefficientDecrease) lerpCoeffitient = 0;
                else lerpCoeffitient -= lerpCoefficientDecrease;
                */
                return lerp(midColor, maxColor, lerpCoeffitient);
            }
    }

    COLORREF colorLerp(double temperature, COLORREF pointColor, COLORREF warmColor, const double lerpCoefficientDecrease /*= 0*/)
    {
        // Checking input:

            assert(0 <= lerpCoefficientDecrease && lerpCoefficientDecrease <= 1);

        // Preparing input:

            if (MAX_TEMPERATURE < temperature)
            {
                temperature = MAX_TEMPERATURE;
            }

            double lerpCoefficient = 3 * log(log(temperature + 1)/(log(10.0)) + 1)/(log(10.0));
            if (lerpCoefficient > 1) lerpCoefficient = 1.0;

            //double lerpCoefficient = temperature/MAX_TEMPERATURE;

            if (lerpCoefficient < lerpCoefficientDecrease) lerpCoefficient = 0;
            else lerpCoefficient -= lerpCoefficientDecrease;

            return lerp(pointColor, warmColor, lerpCoefficient);
    }

//}
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//{ Field
//----------------------------------------------------------------------------

    class Field
    {
        public:

            // Constructor && destructor:

                Field(const char* conductivitiesFileName,
                      const char*      obstaclesFileName,
                      const char*          imageFileName,
                      const size_t width,
                      const size_t height,
                      const double wallConditions,
                      const double emptySpaceConditions,
                      double (*fillingFunction) (const unsigned int x, const unsigned int y));

                ~Field();

            // Functions:

                // Debugging:

                    bool ok() const;

                // Setting conditions:

                    void setWallConditions(const double borderTemperature = 1.0);
                    void setFieldConditions(double (*gradientFunction) (const unsigned int x, const unsigned int y));

                    void editorMode(const unsigned int brushRadius, double brushDeltaTemperature, const unsigned int zoom /*= 1*/);
                    void adjustTemperature(const unsigned int roundX, const unsigned int roundY, const unsigned int radius, const double deltaTemperature);

                // Calculations:

                    void calculate();

                // Rendering:

                    void render(const unsigned int zoom = 1, bool grid = false) const;

        private:

            char** obstacles_;

            double** conductivities_;
            double** temperatures_;

            HDC image_;

            size_t  width_;
            size_t height_;
    };


    //----------------------------------------------------------------------------
    //{ Constructor && destructor:
    //----------------------------------------------------------------------------

        Field::Field(const char* conductivitiesFileName,
                     const char*      obstaclesFileName,
                     const char*          imageFileName,
                     const size_t width,
                     const size_t height,
                     const double wallConditions,
                     const double emptySpaceConditions,
                     double (*fillingFunction) (const unsigned int x, const unsigned int y)) :
            obstacles_      (nullptr),
            conductivities_ (nullptr),
            temperatures_   (nullptr),
            image_          (nullptr),
            width_          (width),
            height_         (height)
        {
            // Checking input:

                assert(obstaclesFileName != nullptr);
                assert(conductivitiesFileName != nullptr);
                assert(imageFileName != nullptr);

            // Creating image:

                image_ = txLoadImage(imageFileName);
                assert(image_);

            // Creating obstacles_ array:

                obstacles_ = (char**) calloc(width_, sizeof(*obstacles_));
                assert(obstacles_);

                for (size_t x = 0; x < width_; x++)
                {
                    assert(0 <= x && x < width_);

                    obstacles_[x] = (char*) calloc(height_, sizeof(*obstacles_[x]));
                    assert(obstacles_[x]);
                }

            // Creating conductivities_ array:

                conductivities_ = (double**) calloc(width_, sizeof(*conductivities_));
                assert(conductivities_);

                for (size_t x = 0; x < width_; x++)
                {
                    assert(0 <= x && x < width_);

                    conductivities_[x] = (double*) calloc(height_, sizeof(*conductivities_[x]));
                    assert(conductivities_[x]);
                }

            // Creating temperatures_ array;

                temperatures_ = (double**) calloc(width_, sizeof(*temperatures_));
                assert(temperatures_);

                for (size_t x = 0; x < width_; x++)
                {
                    assert(0 <= x && x < width_);

                    temperatures_[x] = (double*) calloc(height_, sizeof(*temperatures_[x]));
                    assert(temperatures_[x]);

                    for (size_t y = 0; y < height_; y++)
                    {
                        assert(0 <= y && y < height_);

                        temperatures_[x][y] = emptySpaceConditions;
                    }
                }

            // Filling obstacles_ array:

                HDC obstaclesMap = txLoadImage(obstaclesFileName);
                assert(obstaclesMap);

                for (size_t x = 0; x < width_; x++)
                {
                    assert(0 <= x && x < width_);

                    for (size_t y = 0; y < height_; y++)
                    {
                        assert(0 <= y && y < height_);

                        COLORREF currentColor = GetPixel(obstaclesMap, x, y);

                        obstacles_[x][y] = (currentColor == RGB(  0, 0, 0))?   WALL_TILE :
                                           (currentColor == RGB(255, 0, 0))? BORDER_TILE :
                                                                              EMPTY_TILE;
                    }
                }

                txDeleteDC(obstaclesMap);

            // Filling conductivities_ array:

                HDC conductivitiesMap = txLoadImage(conductivitiesFileName);
                assert(conductivitiesMap);

                for (size_t x = 0; x < width_; x++)
                {
                    assert(0 <= x && x < width_);

                    for (size_t y = 0; y < height_; y++)
                    {
                        assert(0 <= y && y < height_);

                        conductivities_[x][y] = THERMAL_CONDUCTIVITY_COEFFICIENT * lerp(0.0, 1.0, (double) txExtractColor(GetPixel(conductivitiesMap, x, y), TX_RED) / 255);
                    }
                }

                txSetFillColor(TX_BLACK);
                txClear();

                txDeleteDC(conductivitiesMap);

            // Filling temperatures_ array:

                setWallConditions(wallConditions);

                if (fillingFunction != nullptr) setFieldConditions(fillingFunction);

            // Checking output:

                assert(ok());
        }

        Field::~Field()
        {
            assert(ok());

            for (size_t x = 0; x < width_; x++)
            {
                assert(0 <= x && x < width_);

                free(obstacles_[x]);
            }

            free(obstacles_);

            for (size_t x = 0; x < width_; x++)
            {
                assert(0 <= x && x < width_);

                free(conductivities_[x]);
            }

            free(conductivities_);

            for (size_t x = 0; x < width_; x++)
            {
                assert(0 <= x && x < width_);

                free(temperatures_[x]);
            }

            free(temperatures_);

            txDeleteDC(image_);
        }

    //}
    //----------------------------------------------------------------------------


    //----------------------------------------------------------------------------
    //{ Functions
    //----------------------------------------------------------------------------

        //----------------------------------------------------------------------------
        //{ Debugging
        //----------------------------------------------------------------------------

            bool Field::ok() const
            {
                bool everythingOk = true;

                if (obstacles_ == nullptr)
                {
                    everythingOk = false;
                    printf("Field::ok(): Obstacles array is a null pointer.");
                }

                for (size_t x = 0; x < width_; x++)
                {
                    assert(0 <= x && x < width_);

                    if (obstacles_[x] == nullptr)
                    {
                        everythingOk = false;
                        printf("Field::ok(): obstacles_[%02d] is a null pointer.", x);
                    }

                    for (size_t y = 0; y < height_; y++)
                    {
                        assert(0 <= y && y < height_);

                        if (obstacles_[x][y] > 2)
                        {
                            everythingOk = false;
                            printf("Field::ok(): obstacles_[%02d][%02d] is invalid tile type.", x, y);
                        }
                    }
                }

                if (conductivities_ == nullptr)
                {
                    everythingOk = false;
                    printf("Field::ok(): Conductivities array is a null pointer.");
                }

                for (size_t x = 0; x < width_; x++)
                {
                    assert(0 <= x && x < width_);

                    if (conductivities_[x] == nullptr)
                    {
                        everythingOk = false;
                        printf("Field::ok(): conductivities_[%02d] is a null pointer.", x);
                    }
                }

                if (temperatures_ == nullptr)
                {
                    everythingOk = false;
                    printf("Field::ok(): Temperature array is a null pointer.");
                }

                for (size_t x = 0; x < width_; x++)
                {
                    assert(0 <= x && x < width_);

                    if (temperatures_[x] == nullptr)
                    {
                        everythingOk = false;
                        printf("Field::ok(): temperature_[%02d] is a null pointer.", x);
                    }
                }

                if (image_ == nullptr)
                {
                    everythingOk = false;
                    printf("Field::ok(): Image array is a null pointer.");
                }

                if (width_ <= 2)
                {
                    everythingOk = false;
                    printf("Field::ok(): Width is too small (NO SIMULATION HERE).");
                }

                if (height_ <= 2)
                {
                    everythingOk = false;
                    printf("Field::ok(): Height is too small (NO SIMULATION HERE).");
                }

                return everythingOk;
            }

        //}
        //----------------------------------------------------------------------------


        //----------------------------------------------------------------------------
        //{ Conditions setting
        //----------------------------------------------------------------------------

            void Field::setWallConditions(const double borderTemperature /*= 1.0*/)
            {
                // Checking input:

                    assert(ok());

                // Main algorithm:

                    for (size_t x = 0; x < width_; x++)
                    {
                        assert(0 <= x && x < width_);

                        // Upper wall:
                        temperatures_[x][0] = borderTemperature;

                        // Bottom wall:
                        temperatures_[x][height_ - 1] = borderTemperature;
                    }

                    for (size_t y = 0; y < height_; y++)
                    {
                        assert(0 <= y && y < height_);

                        // Left wall:
                        temperatures_[0][y] = borderTemperature;

                        // Right wall:
                        temperatures_[width_ - 1][y] = borderTemperature;
                    }

                    for (size_t x = 1; x < width_ - 1; x++)
                    {
                        assert(1 <= x && x < width_ - 1);

                        for (size_t y = 1; y < height_ - 1; y++)
                        {
                            assert(1 <= y && y < height_ - 1);

                            if (obstacles_[x][y] == BORDER_TILE) temperatures_[x][y] = borderTemperature;
                        }
                    }

                // Checking output;

                    assert(ok());
            }

            void Field::setFieldConditions(double (*gradientFunction) (const unsigned int x, const unsigned int y))
            {
                // Checking input:

                    assert(ok());

                // Main algorithm:

                    for (size_t x = 1; x < width_ - 1; x++)
                    {
                        assert(1 <= x && x < width_ - 1);

                        for (size_t y = 1; y < height_ - 1; y++)
                        {
                            assert(1 <= y && y < height_ - 1);

                            if (obstacles_[x][y] == EMPTY_TILE) temperatures_[x][y] = gradientFunction(x, y);
                        }
                    }

                // Checking output:

                    assert(ok());
            }

            // Hate it
            // Mixed style -___-
            void Field::editorMode(const unsigned int brushRadius, double brushDeltaTemperature, const unsigned int zoom /*= 1*/)
            {
                // Checking input:

                    assert(ok());

                // Main algorithm:

                    txClearConsole();
                    puts("[EDITOR MODE]");

                    while (!GetAsyncKeyState(VK_RSHIFT) && !GetAsyncKeyState(VK_ESCAPE))
                    {
                        if (txMouseButtons() == 1)
                        {
                            adjustTemperature(txMouseX()/zoom, txMouseY()/zoom, brushRadius, brushDeltaTemperature);
                        }

                        if (GetAsyncKeyState('S')) brushDeltaTemperature -= 1;
                        if (GetAsyncKeyState('W')) brushDeltaTemperature += 1;

                        if (GetAsyncKeyState(VK_SPACE))
                        {
                            txClearConsole();
                            txBegin();

                            render(zoom, GetAsyncKeyState('0'));

                            txEnd();
                        }

                        txClearConsole();
                        puts("[EDITOR MODE]");
                        printf("DELTA == %.03f\n", brushDeltaTemperature);

                        txSleep(20);
                    }

                    txClearConsole();
                    puts("[SIMULATION MODE]");

                // Checking output:

                    assert(ok());

            }

            void Field::adjustTemperature(const unsigned int roundX, const unsigned int roundY, const unsigned int radius, const double deltaTemperature)
            {
                // Checking input:

                    assert(ok());

                // Creating resources:

                    unsigned int startX = (roundX < radius)? 0 : roundX - radius;
                    unsigned int startY = (roundY < radius)? 0 : roundY - radius;

                    unsigned int finishX = (roundX + radius <  width_)? roundX + radius :  width_ - 1;
                    unsigned int finishY = (roundY + radius < height_)? roundY + radius : height_ - 1;

                    assert(0 <= startX && startX <  width_);
                    assert(0 <= startY && startY < height_);

                    assert(0 <= finishX && finishX <  width_);
                    assert(0 <= finishY && finishY < height_);

                // Main algorithm:

                    for (size_t x = startX; x < finishX; x++)
                    {
                        for (size_t y = startY; y < finishY; y++)
                        {
                            if (obstacles_[x][y] == EMPTY_TILE)
                            {
                                if (pow((signed) roundX - (signed) x, 2) + pow((signed) roundY - (signed) y, 2) < pow(radius, 2))
                                {
                                    if  (temperatures_[x][y] + deltaTemperature < 0) temperatures_[x][y] = 0;
                                    else temperatures_[x][y] += deltaTemperature;
                                }
                            }
                        }
                    }

            }

        //}
        //----------------------------------------------------------------------------


        //----------------------------------------------------------------------------
        //{ Calculations
        //----------------------------------------------------------------------------

            void Field::calculate()
            {
                // Checking input:

                    assert(ok());

                // Creating resources:

                    if (GetAsyncKeyState(VK_LCONTROL))
                    {
                        txClearConsole();
                        txSetFillColor(TX_BLUE);
                        txSetColor    (TX_BLUE);

                        printf("T[105][150] == %f     \n", temperatures_[105][150]); txCircle(105 * 3, 150 * 3, 6);
                        printf("T[105][160] == %f     \n", temperatures_[105][170]); txCircle(105 * 3, 160 * 3, 6);
                        printf("T[105][165] == %f     \n", temperatures_[105][165]); txCircle(105 * 3, 165 * 3, 6);
                        printf("T[110][175] == %f     \n", temperatures_[110][175]); txCircle(110 * 3, 175 * 3, 6);
                        printf("T[115][180] == %f     \n", temperatures_[115][180]); txCircle(115 * 3, 180 * 3, 6);
                        printf("T[165][190] == %f     \n", temperatures_[165][190]); txCircle(165 * 3, 190 * 3, 6);

                        txSleep(100);
                    }

                    if (txMouseButtons() == 1)
                        printf("Temperature[%02d][%02d] == %.2f     \r",
                               txMouseX()/3, txMouseY()/3, temperatures_[txMouseX()/3][txMouseY()/3] * 10);


                    double** nextTemperatures_ = (double**) calloc(width_, sizeof(*nextTemperatures_));
                    assert(temperatures_);

                    for (size_t x = 0; x < width_; x++)
                    {
                        assert(0 <= x && x < width_);

                        nextTemperatures_[x] = (double*) calloc(height_, sizeof(*nextTemperatures_[x]));
                        assert(nextTemperatures_[x]);
                    }

                // Main algorithm:

                    for (size_t x = 1; x < width_ - 1; x++)
                    {
                        assert(1 <= x && x < width_ - 1);

                        for (size_t y = 1; y < height_ - 1; y++)
                        {
                            assert(1 <= y && y < height_ - 1);

                            if (obstacles_[x][y] == EMPTY_TILE)
                            {
                                double nextTemperature  = (obstacles_[x - 1][y] != WALL_TILE)? temperatures_[x - 1][y] : 0;
                                       nextTemperature += (obstacles_[x + 1][y] != WALL_TILE)? temperatures_[x + 1][y] : 0;

                                       nextTemperature += -4 * temperatures_[x][y];

                                       nextTemperature += (obstacles_[x][y - 1] != WALL_TILE)? temperatures_[x][y - 1] : 0;
                                       nextTemperature += (obstacles_[x][y + 1] != WALL_TILE)? temperatures_[x][y + 1] : 0;

                                       nextTemperature *= conductivities_[x][y];
                                       nextTemperature *= TIME_STEP;
                                       nextTemperature /= SPACE_STEP * SPACE_STEP;

                                       nextTemperature += temperatures_[x][y];

                                nextTemperatures_[x][y] = nextTemperature;
                            }
                        }
                    }

                    for (size_t x = 0; x < width_; x++)
                    {
                        assert(0 <= x && x < width_);

                        free(temperatures_[x]);
                    }

                    free(temperatures_);

                    temperatures_ = nextTemperatures_;

                // Checking input:

                    assert(ok());
            }

        //}
        //----------------------------------------------------------------------------


        //----------------------------------------------------------------------------
        //{ Rendering
        //----------------------------------------------------------------------------

            void Field::render(const unsigned int zoom /*= 1*/, bool grid /*= false*/) const
            {
                // Checking input:

                    assert(ok());

                // Main algorithm:

                    txSetFillColor (TX_BLACK);
                    txClear();

                    for (size_t x = 0; x < width_; x++)
                    {
                        for (size_t y = 0; y < height_; y++)
                        {
                            //COLORREF currentColor = (obstacles_[x][y] == WALL_TILE)? WALL_COLOR : colorLerp(temperatures_[x][y], COLD_COLOR, MID_COLOR, WARM_COLOR);

                            COLORREF currentColor = colorLerp(log(log(temperatures_[x][y] + 1) + 1), GetPixel(image_, x, y), MID_COLOR, WARM_COLOR);

                            if (zoom >= 3)
                            {
                                txSetColor    (currentColor);
                                txSetFillColor(currentColor);

                                txRectangle(x * zoom, y * zoom, (x + 1) * zoom - (int) grid, (y + 1) * zoom - (int) grid);
                            }
                            else
                            {
                                txSetPixel(x * zoom, y * zoom, currentColor);
                            }

                        }
                    }
            }

        //}
        //----------------------------------------------------------------------------

    //}
    //----------------------------------------------------------------------------

//}
//----------------------------------------------------------------------------

