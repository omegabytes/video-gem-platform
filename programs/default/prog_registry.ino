// =====================================================================
// PROGRAM REGISTRY — default platform programs (Video_Gem* by RMNA)
//
// Registers the default programs that ship with the platform.
// A parent repo can overlay this with its own prog_registry.ino.
// =====================================================================

ProgEntry g_programs[] = {
  PROG_ENTRY(0, prog_basic_shapes),
  PROG_ENTRY(1, prog_symmetry),
  PROG_ENTRY_EX(2, prog_color, NULL, PROG_FLAG_OWNS_GLOBALS),
  PROG_ENTRY(3, prog_inputs),
  PROG_ENTRY(9, prog_bitmaps),
  PROG_ENTRY(11, prog_fx),
};
int g_numPrograms = sizeof(g_programs) / sizeof(g_programs[0]);
