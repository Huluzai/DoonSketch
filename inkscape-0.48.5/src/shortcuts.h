#ifndef __SP_SHORTCUTS_H__
#define __SP_SHORTCUTS_H__

/*
 * Keyboard shortcut processing
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

namespace Inkscape {
    class Verb;
    namespace UI {
        namespace View {
            class View;
        }
    }
}

/* We define high-bit mask for packing into single int */

#define SP_SHORTCUT_SHIFT_MASK (1 << 24)
#define SP_SHORTCUT_CONTROL_MASK (1 << 25)
#define SP_SHORTCUT_ALT_MASK (1 << 26)
#define SP_SHORTCUT_MODIFIER_MASK (SP_SHORTCUT_SHIFT_MASK | SP_SHORTCUT_CONTROL_MASK | SP_SHORTCUT_ALT_MASK)

/* Returns true if action was performed */
bool sp_shortcut_invoke (unsigned int shortcut, Inkscape::UI::View::View *view);

Inkscape::Verb * sp_shortcut_get_verb( unsigned int shortcut );

/**
 * Returns the associated shortcut, or GDK_VoidSymbol.
 * @param verb the verb to look up a shortcut for.
 *
 * @return the appropriate shortcut, or GDK_VoidSymbol if no shortcut is found.
 */
unsigned int sp_shortcut_get_primary( Inkscape::Verb * verb );

/**
 * Return the human readable form of the shortcut, or NULL.
 * @param shortcut the shortcut value.
 *
 * @return a human readable form of the shortcut, or NULL (e.g. Shift+Ctrl+F).
 *         Free the returned string with g_free.
 */
char* sp_shortcut_get_label( unsigned int shortcut );

#endif

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
