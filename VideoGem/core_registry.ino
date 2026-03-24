// =====================================================================
// CORE: Program registry — lookup, macro, program count
//
// Infrastructure for the table-driven program dispatch. The actual
// g_programs[] array is defined in prog_registry.ino (stub, default,
// or private overlay depending on the build).
// =====================================================================

#define PROG_ENTRY(slot, prefix) \
  { (slot), prefix##_draw, prefix##_renderHint, prefix##_name, \
    prefix##_character, prefix##_presetName, prefix##_potLabel, \
    NULL, 0 }

#define PROG_ENTRY_EX(slot, prefix, init_fn, fl) \
  { (slot), prefix##_draw, prefix##_renderHint, prefix##_name, \
    prefix##_character, prefix##_presetName, prefix##_potLabel, \
    (init_fn), (fl) }

// g_numPrograms is defined in prog_registry.ino (next to g_programs[])

ProgEntry* progForSlot(int slot) {
  for (int i = 0; i < g_numPrograms; i++) {
    if (g_programs[i].draw && g_programs[i].slot == slot)
      return &g_programs[i];
  }
  return NULL;
}
