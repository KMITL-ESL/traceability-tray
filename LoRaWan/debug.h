#ifndef DEBUG_h
#define DEBUG_h

#define SHOW_DEBUG_CMD
#define SHOW_DEBUG_I2C

#ifdef SHOW_DEBUG_CMD
#define DEBUG_CMD(msg)                \
    do                                \
    {                                 \
        Serial.print("DEBUG CMD : "); \
        Serial.println(msg);          \
    } while (0)
#else
#define DEBUG_CMD(msg) \
    do                 \
    {                  \
    } while (0)
#endif

#ifdef SHOW_DEBUG_I2C
#define DEBUG_I2C(msg)                \
    do                                \
    {                                 \
        Serial.print("DEBUG I2C : "); \
        Serial.println(msg);          \
    } while (0)
#else
#define DEBUG_I2C(msg) \
    do                 \
    {                  \
    } while (0)
#endif

#endif
