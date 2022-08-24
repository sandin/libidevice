#ifndef IDEVICE_MACRO_DEF_H
#define IDEVICE_MACRO_DEF_H

// DEBUG 
#ifdef NDEBUG
#define IDEVICE_DEBUG 0
#else
#define IDEVICE_DEBUG 1
#endif

// LOG 
#define IDEVICE_LOG_LEVEL 2
#if IDEVICE_LOG_LEVEL >= 0
#define IDEVICE_LOG_E(fmt, ...) \
  fprintf(stderr, "[ERROR] %s:%d:%s(): " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
#define IDEVICE_LOG_E(fmt, ...)
#endif
#if IDEVICE_LOG_LEVEL >= 1
#define IDEVICE_LOG_I(fmt, ...) \
  printf("[INFO] %s:%d:%s(): " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
#define IDEVICE_LOG_I(fmt, ...)
#endif
#if IDEVICE_LOG_LEVEL >= 2
#define IDEVICE_LOG_D(fmt, ...) \
  printf("[DEBUG] %s:%d:%s(): " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
#define IDEVICE_LOG_D(fmt, ...)
#endif
#if IDEVICE_LOG_LEVEL >= 3
#define IDEVICE_LOG_V(fmt, ...) \
  printf("[VERBOSE] %s:%d:%s(): " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
#define IDEVICE_LOG_V(fmt, ...)
#endif

// ASSERT
#define IDEVICE_ASSERT(exp, fmt, ...) if (!(exp)) { LOG_ERROR(fmt, ##__VA_ARGS__); } assert(exp)


#endif // IDEVICE_MACRO_SCOPE_H
