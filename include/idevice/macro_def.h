/**
 * NOTE: there two rules to import this file:
 * 1. IT IS OK to import this file in any "*.cpp" files.
 * 2. TRY NOT TO import this file in any "*.h/hpp" header files. If you have to do it,  please import the "macro_undef.h" file
 *   at the end of the header file to avoid bringing these macro definitions to other files.
 */
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

// MEMORY
#define IDEVICE_MEM_ALIGN(v, a) (((v) + (a)-1) & ~((a)-1))

// ASSERT
#define IDEVICE_ASSERT(exp, fmt, ...) if (!(exp)) { LOG_ERROR(fmt, ##__VA_ARGS__); } assert(exp)

// HELPER
#define IDEVICE_ATOMIC_SET_MAX(atomic_value, max_value) \
  uint32_t prev_value = atomic_value; \
  while (prev_value < max_value && !atomic_value.compare_exchange_weak(prev_value, max_value)) {}

#define IDEVICE_START_THREAD(thread_var, thread_func, stop_flag) \
  stop_flag.store(true, std::memory_order_release); \
  thread_var = std::make_unique<std::thread>(std::bind(thread_func, this));

#define IDEVICE_STOP_THREAD(thread_var, stop_flag, await) \
  if (stop_flag.load(std::memory_order_acquire)) { \
    stop_flag.store(false, std::memory_order_release); \
    if (await) { \
      thread_var->join(); \
    } \
  }

#define IDEVICE_DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&) = delete;      \
  void operator=(const TypeName&) = delete

// DTXMESSAGE
#define IDEVICE_DUMP_DTXMESSAGE_HEADER(header) \
  printf("==============\n"); \
  printf("magic: %x\n", header.magic); \
  printf("message_header_size: %d\n", header.message_header_size); \
  printf("fragment_index: %d\n", header.fragment_index); \
  printf("fragment_count: %d\n", header.fragment_count); \
  printf("length: %d\n", header.length); \
  printf("identifier: %d\n", header.identifier); \
  printf("conversation_index: %d\n", header.conversation_index); \
  printf("channel_code: %d\n", header.channel_code); \
  printf("expects_reply: %d\n", header.expects_reply); \
  printf("==============\n"); \

#define IDEVICE_DTXMESSAGE_IDENTIFIER(channel_code, msg_identifier) static_cast<uint64_t>(channel_code) << 32 | msg_identifier

#endif // IDEVICE_MACRO_SCOPE_H
