#ifdef IDEVICE_MACRO_DEF_H // make sure the "macro_def.h" file has been imported,
#undef IDEVICE_MACRO_DEF_H // and undo everything that was done in the "macro_def.h" file

// DEBUG
#undef IDEVICE_DEBUG

// LOG
#undef IDEVICE_LOG_LEVEL
#undef IDEVICE_LOG_E
#undef IDEVICE_LOG_I
#undef IDEVICE_LOG_D
#undef IDEVICE_LOG_V

// MEMORY
#undef IDEVICE_MEM_ALIGN

// ASSERT
#undef IDEVICE_ASSERT

// HELPER
#undef IDEVICE_ATOMIC_SET_MAX
#undef IDEVICE_START_THREAD
#undef IDEVICE_STOP_THREAD
#undef IDEVICE_DISALLOW_COPY_AND_ASSIGN

// DTXMESSAGE
#undef IDEVICE_DUMP_DTXMESSAGE_HEADER
#undef IDEVICE_DTXMESSAGE_IDENTIFIER

#endif // IDEVICE_MACRO_DEF_H
