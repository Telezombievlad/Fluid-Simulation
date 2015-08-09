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

    const double THERMAL_CONDUCTIVITY = 1;

    const double MAX_TEMPERATURE = 11000/*K*/;

    const double  TIME_STEP = 10;
    const double SPACE_STEP = 8;

    COLORREF COLD_COLOR = RGB(  0,   0, 255);
    COLORREF  MID_COLOR = RGB(  0, 255,   0);
    COLORREF WARM_COLOR = RGB(255,   0,   0);

//}
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//{ Function prototypes:
//----------------------------------------------------------------------------

    double lerp(const double a, const double b, const double k);
    COLORREF colorLerp(double temperature, COLORREF minColor, COLORREF midColor, COLORREF maxColor);

//}
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//{ Field
//----------------------------------------------------------------------------

    class Field
    {
        public:

            // Constructor && destructor:

                Field(const size_t width, const size_t height, double (*fillingFunction) (const unsigned int x, const unsigned int y));

                ~Field();

            // Functions:

                // Debugging:

                    bool ok() const;

                // Setting conditions:

                    void setWallConditions(const double wallTemperature = 1.0);
                    void setFieldConditions(double (*gradientFunction) (const unsigned int x, const unsigned int y));

                    void setFieldConditionsManually(const unsigned int radius, double deltaTemperature);
                    void adjustTemperature(const unsigned int roundX, const unsigned int roundY, const unsigned int radius, const double deltaTemperature);

                // Calculations:

                    void calculate();

                // Rendering:

                    void render() const;

        private:

            double** temperatures_;

            size_t  width_;
            size_t height_;
    };


    //----------------------------------------------------------------------------
    //{ Constructor && destructor:
    //----------------------------------------------------------------------------

        Field::Field(const size_t width, const size_t height, double (*fillingFunction) (const unsigned int x, const unsigned int y)) :
            temperatures_ (nullptr),
            width_        (width),
            height_       (height)
        {
            assert(fillingFunction != nullptr);

            temperatures_ = (double**) calloc(width_, sizeof(*temperatures_));
            assert(temperatures_);

            for (size_t x = 0; x < width_; x++)
            {
                assert(0 <= x && x < width_);

                temperatures_[x] = (double*) calloc(height_, sizeof(*temperatures_[x]));
                assert(temperatures_[x]);
            }

            setWallConditions(1.0);

            setFieldConditions(fillingFunction);

            assert(ok());
        }

        Field::~Field()
        {
            assert(ok());

            for (size_t x = 0; x < width_; x++)
            {
                assert(0 <= x && x < width_);

                free(temperatures_[x]);
            }

            free(temperatures_);
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

            void Field::setWallConditions(const double wallTemperature /*= 1.0*/)
            {
                // Checking input:

                    assert(ok());

                // Main algorithm:

                    for (size_t x = 0; x < width_; x++)
                    {
                        assert(0 <= x && x < width_);

                        // Upper wall:
                        temperatures_[x][0] = wallTemperature;

                        // Bottom wall:
                        temperatures_[x][height_ - 1] = wallTemperature;
                    }

                    for (size_t y = 0; y < height_; y++)
                    {
                        assert(0 <= y && y < height_);

                        // Left wall:
                        temperatures_[0][y] = wallTemperature;

                        // Right wall:
                        temperatures_[width_ - 1][y] = wallTemperature;
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

                            temperatures_[x][y] = gradientFunction(x, y);
                        }
                    }

                // Checking output:

                    assert(ok());
            }

            // Hate it
            // Mixed style -___-
            void Field::setFieldConditionsManually(const unsigned int radius, double deltaTemperature)
            {
                // Checking input:

                    assert(ok());

                // Main algorithm:

                    txClearConsole();
                    puts("[EDITOR MODE]");

                    while (!GetAsyncKeyState(VK_RSHIFT) && !GetAsyncKeyState(VK_ESCAPE))
                    {
                        if (GetAsyncKeyState(VK_LBUTTON))
                        {
                            adjustTemperature(txMouseX(), txMouseY(), radius, deltaTemperature);
                        }

                        if (GetAsyncKeyState('S')) deltaTemperature -= 1;
                        if (GetAsyncKeyState('W')) deltaTemperature += 1;

                        if (GetAsyncKeyState(VK_SPACE))
                        {
                            txClearConsole();
                            txBegin();

                            render();

                            txEnd();
                        }

                        txClearConsole();
                        puts("[EDITOR MODE]");
                        printf("DELTA == %.03f\n", deltaTemperature);

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
                            if (pow((signed) roundX - (signed) x, 2) + pow((signed) roundY - (signed) y, 2) < pow(radius, 2))
                            {
                                if  (temperatures_[x][y] + deltaTemperature < 0) temperatures_[x][y] = 0;
                                else temperatures_[x][y] += deltaTemperature;
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

                            double nextTemperature  =      temperatures_[x - 1][y];
                                   nextTemperature += -2 * temperatures_[x    ][y];
                                   nextTemperature +=      temperatures_[x + 1][y];

                                   nextTemperature +=      temperatures_[x][y - 1];
                                   nextTemperature += -2 * temperatures_[x][y    ];
                                   nextTemperature +=      temperatures_[x][y + 1];

                                   nextTemperature *= THERMAL_CONDUCTIVITY;
                                   nextTemperature *= TIME_STEP;
                                   nextTemperature /= SPACE_STEP * SPACE_STEP;

                                   nextTemperature += temperatures_[x][y];

                            nextTemperatures_[x][y] = nextTemperature;
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

            void Field::render() const
            {
                // Checking input:

                    assert(ok());

                // Main algorithm:

                    for (size_t x = 0; x < width_; x++)
                    {
                        for (size_t y = 0; y < height_; y++)
                        {
                            txSetPixel(x, y, colorLerp(temperatures_[x][y], COLD_COLOR, MID_COLOR, WARM_COLOR));
                        }
                    }
            }

        //}
        //----------------------------------------------------------------------------

    //}
    //----------------------------------------------------------------------------

//}
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//{ Additional functions
//----------------------------------------------------------------------------

    inline double lerp(const double a, const double b, const double k)
    {
        assert(0 <= k && k <= 1.0);

        return a + k * (b - a);
    }

    COLORREF colorLerp(double temperature, COLORREF minColor, COLORREF midColor, COLORREF maxColor)
    {
        // Checking input:

            assert(0 < MAX_TEMPERATURE);

        // Preparing input:

            if (MAX_TEMPERATURE < temperature)
            {
                //printf("too big temperature == %f\n", temperature);

                temperature = MAX_TEMPERATURE;
            }

            if (temperature < MAX_TEMPERATURE/2)
            {
                double lerpCoefficient = temperature/(MAX_TEMPERATURE/2);

                return RGB((unsigned char) lerp(txExtractColor(minColor, TX_RED  ), txExtractColor(midColor, TX_RED  ), lerpCoefficient),
                           (unsigned char) lerp(txExtractColor(minColor, TX_GREEN), txExtractColor(midColor, TX_GREEN), lerpCoefficient),
                           (unsigned char) lerp(txExtractColor(minColor, TX_BLUE ), txExtractColor(midColor, TX_BLUE ), lerpCoefficient));
            }
            else
            {
                double lerpCoefficient = (temperature - MAX_TEMPERATURE/2)/(MAX_TEMPERATURE/2);

                return RGB((unsigned char) lerp(txExtractColor(midColor, TX_RED  ), txExtractColor(maxColor, TX_RED  ), lerpCoefficient),
                           (unsigned char) lerp(txExtractColor(midColor, TX_GREEN), txExtractColor(maxColor, TX_GREEN), lerpCoefficient),
                           (unsigned char) lerp(txExtractColor(midColor, TX_BLUE ), txExtractColor(maxColor, TX_BLUE ), lerpCoefficient));
            }
    }

//}
//----------------------------------------------------------------------------

