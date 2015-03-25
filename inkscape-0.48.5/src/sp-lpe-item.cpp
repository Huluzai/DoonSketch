#define __SP_LPE_ITEM_CPP__

/** \file
 * Base class for live path effect items
 */
/*
 * Authors:
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Bastien Bouclet <bgkweb@gmail.com>
 *
 * Copyright (C) 2008 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "live_effects/effect.h"
#include "live_effects/lpe-path_length.h"
#include "live_effects/lpeobject.h"
#include "live_effects/lpeobject-reference.h"

#include "sp-path.h"
#include "sp-item-group.h"
#include "streq.h"
#include "macros.h"
#include "attributes.h"
#include "sp-lpe-item.h"
#include "xml/repr.h"
#include "uri.h"
#include "message-stack.h"
#include "inkscape.h"
#include "desktop.h"
#include "shape-editor.h"

#include <algorithm>

/* LPEItem base class */

static void sp_lpe_item_class_init(SPLPEItemClass *klass);
static void sp_lpe_item_init(SPLPEItem *lpe_item);
static void sp_lpe_item_finalize(GObject *object);

static void sp_lpe_item_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_lpe_item_release(SPObject *object);
static void sp_lpe_item_set(SPObject *object, unsigned int key, gchar const *value);
static void sp_lpe_item_update(SPObject *object, SPCtx *ctx, guint flags);
static void sp_lpe_item_modified (SPObject *object, unsigned int flags);
static Inkscape::XML::Node *sp_lpe_item_write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags);

static void sp_lpe_item_child_added (SPObject * object, Inkscape::XML::Node * child, Inkscape::XML::Node * ref);
static void sp_lpe_item_remove_child (SPObject * object, Inkscape::XML::Node * child);

static void sp_lpe_item_enable_path_effects(SPLPEItem *lpeitem, bool enable);

static void lpeobject_ref_modified(SPObject *href, guint flags, SPLPEItem *lpeitem);

static void sp_lpe_item_create_original_path_recursive(SPLPEItem *lpeitem);
static void sp_lpe_item_cleanup_original_path_recursive(SPLPEItem *lpeitem);
typedef std::list<std::string> HRefList;
static std::string patheffectlist_write_svg(PathEffectList const & list);
static std::string hreflist_write_svg(HRefList const & list);

static SPItemClass *parent_class;

GType
sp_lpe_item_get_type()
{
    static GType lpe_item_type = 0;

    if (!lpe_item_type) {
        GTypeInfo lpe_item_info = {
            sizeof(SPLPEItemClass),
            NULL, NULL,
            (GClassInitFunc) sp_lpe_item_class_init,
            NULL, NULL,
            sizeof(SPLPEItem),
            16,
            (GInstanceInitFunc) sp_lpe_item_init,
            NULL,    /* value_table */
        };
        lpe_item_type = g_type_register_static(SP_TYPE_ITEM, "SPLPEItem", &lpe_item_info, (GTypeFlags)0);
    }
    return lpe_item_type;
}

static void
sp_lpe_item_class_init(SPLPEItemClass *klass)
{
    GObjectClass *gobject_class;
    SPObjectClass *sp_object_class;

    gobject_class = (GObjectClass *) klass;
    sp_object_class = (SPObjectClass *) klass;
    parent_class = (SPItemClass *)g_type_class_peek_parent (klass);

    gobject_class->finalize = sp_lpe_item_finalize;

    sp_object_class->build = sp_lpe_item_build;
    sp_object_class->release = sp_lpe_item_release;
    sp_object_class->set = sp_lpe_item_set;
    sp_object_class->update = sp_lpe_item_update;
    sp_object_class->modified = sp_lpe_item_modified;
    sp_object_class->write = sp_lpe_item_write;
    sp_object_class->child_added = sp_lpe_item_child_added;
    sp_object_class->remove_child = sp_lpe_item_remove_child;

    klass->update_patheffect = NULL;
}

static void
sp_lpe_item_init(SPLPEItem *lpeitem)
{
    lpeitem->path_effects_enabled = 1;

    lpeitem->path_effect_list = new PathEffectList();
    lpeitem->current_path_effect = NULL;

    lpeitem->lpe_modified_connection_list = new std::list<sigc::connection>();
}

static void
sp_lpe_item_finalize(GObject *object)
{
    if (((GObjectClass *) (parent_class))->finalize) {
        (* ((GObjectClass *) (parent_class))->finalize)(object);
    }
}

/**
 * Reads the Inkscape::XML::Node, and initializes SPLPEItem variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
static void
sp_lpe_item_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    sp_object_read_attr(object, "inkscape:path-effect");

    if (((SPObjectClass *) parent_class)->build) {
        ((SPObjectClass *) parent_class)->build(object, document, repr);
    }
}

/**
 * Drops any allocated memory.
 */
static void
sp_lpe_item_release(SPObject *object)
{
    SPLPEItem *lpeitem = (SPLPEItem *) object;

    // disconnect all modified listeners:
    for (std::list<sigc::connection>::iterator mod_it = lpeitem->lpe_modified_connection_list->begin();
         mod_it != lpeitem->lpe_modified_connection_list->end(); ++mod_it)
    {
        mod_it->disconnect();
    }
    delete lpeitem->lpe_modified_connection_list;
    lpeitem->lpe_modified_connection_list = NULL;

    PathEffectList::iterator it = lpeitem->path_effect_list->begin();
    while ( it != lpeitem->path_effect_list->end() ) {
        // unlink and delete all references in the list
        (*it)->unlink();
        delete *it;
        it = lpeitem->path_effect_list->erase(it);
    }
    // delete the list itself
    delete lpeitem->path_effect_list;
    lpeitem->path_effect_list = NULL;

    if (((SPObjectClass *) parent_class)->release)
        ((SPObjectClass *) parent_class)->release(object);
}

/**
 * Sets a specific value in the SPLPEItem.
 */
static void
sp_lpe_item_set(SPObject *object, unsigned int key, gchar const *value)
{
    SPLPEItem *lpeitem = (SPLPEItem *) object;

    switch (key) {
        case SP_ATTR_INKSCAPE_PATH_EFFECT:
            {
                lpeitem->current_path_effect = NULL;

                // Disable the path effects while populating the LPE list
                 sp_lpe_item_enable_path_effects(lpeitem, false);

                // disconnect all modified listeners:
                for ( std::list<sigc::connection>::iterator mod_it = lpeitem->lpe_modified_connection_list->begin();
                      mod_it != lpeitem->lpe_modified_connection_list->end();
                      ++mod_it)
                {
                    mod_it->disconnect();
                }
                lpeitem->lpe_modified_connection_list->clear();
                // Clear the path effect list
                PathEffectList::iterator it = lpeitem->path_effect_list->begin();
                while ( it != lpeitem->path_effect_list->end() )
                {
                    (*it)->unlink();
                    delete *it;
                    it = lpeitem->path_effect_list->erase(it);
                }

                // Parse the contents of "value" to rebuild the path effect reference list
                if ( value ) {
                    std::istringstream iss(value);
                    std::string href;
                    while (std::getline(iss, href, ';'))
                    {
                        Inkscape::LivePathEffect::LPEObjectReference *path_effect_ref = new Inkscape::LivePathEffect::LPEObjectReference(object);
                        try {
                            path_effect_ref->link(href.c_str());
                        } catch (Inkscape::BadURIException e) {
                            g_warning("BadURIException when trying to find LPE: %s", e.what());
                            path_effect_ref->unlink();
                            delete path_effect_ref;
                            path_effect_ref = NULL;
                        }

                        lpeitem->path_effect_list->push_back(path_effect_ref);
                        if ( path_effect_ref->lpeobject && path_effect_ref->lpeobject->get_lpe() ) {
                            // connect modified-listener
                            lpeitem->lpe_modified_connection_list->push_back(
                                                path_effect_ref->lpeobject->connectModified(sigc::bind(sigc::ptr_fun(&lpeobject_ref_modified), lpeitem)) );
                        } else {
                            // something has gone wrong in finding the right patheffect.
                            g_warning("Unknown LPE type specified, LPE stack effectively disabled");
                            // keep the effect in the lpestack, so the whole stack is effectively disabled but maintained
                        }
                    }
                }

                sp_lpe_item_enable_path_effects(lpeitem, true);
            }
            break;
        default:
            if (((SPObjectClass *) parent_class)->set) {
                ((SPObjectClass *) parent_class)->set(object, key, value);
            }
            break;
    }
}

/**
 * Receives update notifications.
 */
static void
sp_lpe_item_update(SPObject *object, SPCtx *ctx, guint flags)
{
    if (((SPObjectClass *) parent_class)->update) {
        ((SPObjectClass *) parent_class)->update(object, ctx, flags);
    }

    // update the helperpaths of all LPEs applied to the item
    // TODO: re-add for the new node tool
}

/**
 * Sets modified flag for all sub-item views.
 */
static void
sp_lpe_item_modified (SPObject *object, unsigned int flags)
{
    if (SP_IS_GROUP(object) && (flags & SP_OBJECT_MODIFIED_FLAG) && (flags & SP_OBJECT_USER_MODIFIED_FLAG_B)) {
        sp_lpe_item_update_patheffect(SP_LPE_ITEM(object), true, true);
    }

    if (((SPObjectClass *) (parent_class))->modified) {
        (* ((SPObjectClass *) (parent_class))->modified) (object, flags);
    }
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
static Inkscape::XML::Node *
sp_lpe_item_write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    SPLPEItem *lpeitem = (SPLPEItem *) object;

    if (flags & SP_OBJECT_WRITE_EXT) {
        if ( sp_lpe_item_has_path_effect(lpeitem) ) {
            std::string href = patheffectlist_write_svg(*lpeitem->path_effect_list);
            repr->setAttribute("inkscape:path-effect", href.c_str());
        } else {
            repr->setAttribute("inkscape:path-effect", NULL);
        }
    }

    if (((SPObjectClass *)(parent_class))->write) {
        ((SPObjectClass *)(parent_class))->write(object, xml_doc, repr, flags);
    }

    return repr;
}

/**
 * returns true when LPE was successful.
 */
bool sp_lpe_item_perform_path_effect(SPLPEItem *lpeitem, SPCurve *curve) {
    if (!lpeitem) return false;
    if (!curve) return false;

    if (sp_lpe_item_has_path_effect(lpeitem) && sp_lpe_item_path_effects_enabled(lpeitem)) {
        for (PathEffectList::iterator it = lpeitem->path_effect_list->begin(); it != lpeitem->path_effect_list->end(); ++it)
        {
            LivePathEffectObject *lpeobj = (*it)->lpeobject;
            if (!lpeobj) {
                /** \todo Investigate the cause of this.
                 * For example, this happens when copy pasting an object with LPE applied. Probably because the object is pasted while the effect is not yet pasted to defs, and cannot be found.
                 */
                g_warning("sp_lpe_item_perform_path_effect - NULL lpeobj in list!");
                return false;
            }
            Inkscape::LivePathEffect::Effect *lpe = lpeobj->get_lpe();
            if (!lpe) {
                /** \todo Investigate the cause of this.
                 * Not sure, but I think this can happen when an unknown effect type is specified...
                 */
                g_warning("sp_lpe_item_perform_path_effect - lpeobj with invalid lpe in the stack!");
                return false;
            }

            if (lpe->isVisible()) {
                if (lpe->acceptsNumClicks() > 0 && !lpe->isReady()) {
                    // if the effect expects mouse input before being applied and the input is not finished
                    // yet, we don't alter the path
                    return false;
                }

                // Groups have their doBeforeEffect called elsewhere
                if (!SP_IS_GROUP(lpeitem)) {
                    lpe->doBeforeEffect(lpeitem);
                }

                try {
                    lpe->doEffect(curve);
                }
                catch (std::exception & e) {
                    g_warning("Exception during LPE %s execution. \n %s", lpe->getName().c_str(), e.what());
                    if (SP_ACTIVE_DESKTOP && SP_ACTIVE_DESKTOP->messageStack()) {
                        SP_ACTIVE_DESKTOP->messageStack()->flash( Inkscape::WARNING_MESSAGE,
                                        _("An exception occurred during execution of the Path Effect.") );
                    }
                    return false;
                }
            }
        }
    }

    return true;
}

/**
 * Calls any registered handlers for the update_patheffect action
 */
void
sp_lpe_item_update_patheffect (SPLPEItem *lpeitem, bool wholetree, bool write)
{
#ifdef SHAPE_VERBOSE
    g_message("sp_lpe_item_update_patheffect: %p\n", lpeitem);
#endif
    g_return_if_fail (lpeitem != NULL);
    g_return_if_fail (SP_IS_LPE_ITEM (lpeitem));

    if (!sp_lpe_item_path_effects_enabled(lpeitem))
        return;

    // TODO: hack! this will be removed when path length measuring is reimplemented in a better way
    PathEffectList lpelist = sp_lpe_item_get_effect_list(lpeitem);
    std::list<Inkscape::LivePathEffect::LPEObjectReference *>::iterator i;
    for (i = lpelist.begin(); i != lpelist.end(); ++i) {
        if ((*i)->lpeobject) {
            Inkscape::LivePathEffect::Effect *lpe = (*i)->lpeobject->get_lpe();
            if (dynamic_cast<Inkscape::LivePathEffect::LPEPathLength *>(lpe)) {
                if (!lpe->isVisible()) {
                    // we manually disable text for LPEPathLength
                    // use static_cast, because we already checked for the right type above
                    static_cast<Inkscape::LivePathEffect::LPEPathLength *>(lpe)->hideCanvasText();
                }
            }
        }
    }

    SPLPEItem *top;

    if (wholetree) {
        SPObject *prev_parent = lpeitem;
        SPObject *parent = prev_parent->parent;
        while (parent && SP_IS_LPE_ITEM(parent) && sp_lpe_item_has_path_effect_recursive(SP_LPE_ITEM(parent))) {
            prev_parent = parent;
            parent = prev_parent->parent;
        }
        top = SP_LPE_ITEM(prev_parent);
    }
    else {
        top = lpeitem;
    }

    if (SP_LPE_ITEM_CLASS (G_OBJECT_GET_CLASS (top))->update_patheffect) {
        SP_LPE_ITEM_CLASS (G_OBJECT_GET_CLASS (top))->update_patheffect (top, write);
    }
}

/**
 * Gets called when any of the lpestack's lpeobject repr contents change: i.e. parameter change in any of the stacked LPEs
 */
static void
lpeobject_ref_modified(SPObject */*href*/, guint /*flags*/, SPLPEItem *lpeitem)
{
#ifdef SHAPE_VERBOSE
    g_message("lpeobject_ref_modified");
#endif
    sp_lpe_item_update_patheffect (lpeitem, true, true);
}

static void
sp_lpe_item_create_original_path_recursive(SPLPEItem *lpeitem)
{
    if (SP_IS_GROUP(lpeitem)) {
        GSList const *item_list = sp_item_group_item_list(SP_GROUP(lpeitem));
        for ( GSList const *iter = item_list; iter; iter = iter->next ) {
            SPObject *subitem = static_cast<SPObject *>(iter->data);
            if (SP_IS_LPE_ITEM(subitem)) {
                sp_lpe_item_create_original_path_recursive(SP_LPE_ITEM(subitem));
            }
        }
    }
    else if (SP_IS_PATH(lpeitem)) {
        Inkscape::XML::Node *pathrepr = SP_OBJECT_REPR(lpeitem);
        if ( !pathrepr->attribute("inkscape:original-d") ) {
            pathrepr->setAttribute("inkscape:original-d", pathrepr->attribute("d"));
        }
    }
}

static void
sp_lpe_item_cleanup_original_path_recursive(SPLPEItem *lpeitem)
{
    if (SP_IS_GROUP(lpeitem)) {
        GSList const *item_list = sp_item_group_item_list(SP_GROUP(lpeitem));
        for ( GSList const *iter = item_list; iter; iter = iter->next ) {
            SPObject *subitem = static_cast<SPObject *>(iter->data);
            if (SP_IS_LPE_ITEM(subitem)) {
                sp_lpe_item_cleanup_original_path_recursive(SP_LPE_ITEM(subitem));
            }
        }
    }
    else if (SP_IS_PATH(lpeitem)) {
        Inkscape::XML::Node *repr = SP_OBJECT_REPR(lpeitem);
        if (!sp_lpe_item_has_path_effect_recursive(lpeitem)
                && repr->attribute("inkscape:original-d")) {
            repr->setAttribute("d", repr->attribute("inkscape:original-d"));
            repr->setAttribute("inkscape:original-d", NULL);
        }
        else {
            sp_lpe_item_update_patheffect(lpeitem, true, true);
        }
    }
}

void sp_lpe_item_add_path_effect(SPLPEItem *lpeitem, gchar *value, bool reset)
{
    if (value) {
        // Apply the path effects here because in the casse of a group, lpe->resetDefaults
        // needs that all the subitems have their effects applied
        sp_lpe_item_update_patheffect(lpeitem, false, true);

        // Disable the path effects while preparing the new lpe
        sp_lpe_item_enable_path_effects(lpeitem, false);

        // Add the new reference to the list of LPE references
        HRefList hreflist;
        for (PathEffectList::const_iterator it = lpeitem->path_effect_list->begin(); it != lpeitem->path_effect_list->end(); ++it)
        {
            hreflist.push_back( std::string((*it)->lpeobject_href) );
        }
        hreflist.push_back( std::string(value) );
        std::string hrefs = hreflist_write_svg(hreflist);

        SP_OBJECT_REPR(lpeitem)->setAttribute("inkscape:path-effect", hrefs.c_str());

        // make sure there is an original-d for paths!!!
        sp_lpe_item_create_original_path_recursive(lpeitem);

        LivePathEffectObject *lpeobj = lpeitem->path_effect_list->back()->lpeobject;
        if (lpeobj && lpeobj->get_lpe()) {
            Inkscape::LivePathEffect::Effect *lpe = lpeobj->get_lpe();
            // Ask the path effect to reset itself if it doesn't have parameters yet
            if (reset) {
                // has to be called when all the subitems have their lpes applied
                lpe->resetDefaults(lpeitem);
            }

            // perform this once when the effect is applied
            lpe->doOnApply(SP_LPE_ITEM(lpeitem));

            // indicate that all necessary preparations are done and the effect can be performed
            lpe->setReady();
        }

        //Enable the path effects now that everything is ready to apply the new path effect
        sp_lpe_item_enable_path_effects(lpeitem, true);

        // Apply the path effect
        sp_lpe_item_update_patheffect(lpeitem, true, true);
    }
}

void sp_lpe_item_add_path_effect(SPLPEItem *lpeitem, LivePathEffectObject * new_lpeobj)
{
    const gchar * repr_id = SP_OBJECT_REPR(new_lpeobj)->attribute("id");
    gchar *hrefstr = g_strdup_printf("#%s", repr_id);
    sp_lpe_item_add_path_effect(lpeitem, hrefstr, false);
    g_free(hrefstr);
}

void sp_lpe_item_remove_current_path_effect(SPLPEItem *lpeitem, bool keep_paths)
{
    Inkscape::LivePathEffect::LPEObjectReference* lperef = sp_lpe_item_get_current_lpereference(lpeitem);
    if (!lperef)
        return;

    PathEffectList new_list = *lpeitem->path_effect_list;
    new_list.remove(lperef); //current lpe ref is always our 'own' pointer from the path_effect_list
    std::string r = patheffectlist_write_svg(new_list);

    if (!r.empty()) {
        SP_OBJECT_REPR(lpeitem)->setAttribute("inkscape:path-effect", r.c_str());
    } else {
        SP_OBJECT_REPR(lpeitem)->setAttribute("inkscape:path-effect", NULL);
    }

    if (!keep_paths) {
        sp_lpe_item_cleanup_original_path_recursive(lpeitem);
    }
}

void sp_lpe_item_remove_all_path_effects(SPLPEItem *lpeitem, bool keep_paths)
{
    SP_OBJECT_REPR(lpeitem)->setAttribute("inkscape:path-effect", NULL);

    if (!keep_paths) {
        sp_lpe_item_cleanup_original_path_recursive(lpeitem);
    }
}

void sp_lpe_item_down_current_path_effect(SPLPEItem *lpeitem)
{
    Inkscape::LivePathEffect::LPEObjectReference* lperef = sp_lpe_item_get_current_lpereference(lpeitem);
    if (!lperef)
        return;

    PathEffectList new_list = *lpeitem->path_effect_list;
    PathEffectList::iterator cur_it = find( new_list.begin(), new_list.end(), lperef );
    if (cur_it != new_list.end()) {
        PathEffectList::iterator down_it = cur_it;
        down_it++;
        if (down_it != new_list.end()) { // perhaps current effect is already last effect
            std::iter_swap(cur_it, down_it);
        }
    }
    std::string r = patheffectlist_write_svg(new_list);
    SP_OBJECT_REPR(lpeitem)->setAttribute("inkscape:path-effect", r.c_str());

    sp_lpe_item_cleanup_original_path_recursive(lpeitem);
}

void sp_lpe_item_up_current_path_effect(SPLPEItem *lpeitem)
{
    Inkscape::LivePathEffect::LPEObjectReference* lperef = sp_lpe_item_get_current_lpereference(lpeitem);
    if (!lperef)
        return;

    PathEffectList new_list = *lpeitem->path_effect_list;
    PathEffectList::iterator cur_it = find( new_list.begin(), new_list.end(), lperef );
    if (cur_it != new_list.end() && cur_it != new_list.begin()) {
        PathEffectList::iterator up_it = cur_it;
        up_it--;
        std::iter_swap(cur_it, up_it);
    }
    std::string r = patheffectlist_write_svg(new_list);

    SP_OBJECT_REPR(lpeitem)->setAttribute("inkscape:path-effect", r.c_str());

    sp_lpe_item_cleanup_original_path_recursive(lpeitem);
}

/** used for shapes so they can see if they should also disable shape calculation and read from d= */
bool sp_lpe_item_has_broken_path_effect(SPLPEItem *lpeitem)
{
    if (lpeitem->path_effect_list->empty())
        return false;

    // go through the list; if some are unknown or invalid, return true
    PathEffectList effect_list =  sp_lpe_item_get_effect_list(lpeitem);
    for (PathEffectList::iterator it = effect_list.begin(); it != effect_list.end(); it++)
    {
        LivePathEffectObject *lpeobj = (*it)->lpeobject;
        if (!lpeobj || !lpeobj->get_lpe())
            return true;
    }

    return false;
}


bool sp_lpe_item_has_path_effect(SPLPEItem *lpeitem)
{
    if (lpeitem->path_effect_list->empty())
        return false;

    // go through the list; if some are unknown or invalid, we are not an LPE item!
    PathEffectList effect_list =  sp_lpe_item_get_effect_list(lpeitem);
    for (PathEffectList::iterator it = effect_list.begin(); it != effect_list.end(); it++)
    {
        LivePathEffectObject *lpeobj = (*it)->lpeobject;
        if (!lpeobj || !lpeobj->get_lpe())
            return false;
    }

    return true;
}

bool sp_lpe_item_has_path_effect_recursive(SPLPEItem *lpeitem)
{
    SPObject *parent = lpeitem->parent;
    if (parent && SP_IS_LPE_ITEM(parent)) {
        return sp_lpe_item_has_path_effect(lpeitem) || sp_lpe_item_has_path_effect_recursive(SP_LPE_ITEM(parent));
    }
    else {
        return sp_lpe_item_has_path_effect(lpeitem);
    }
}

Inkscape::LivePathEffect::Effect*
sp_lpe_item_has_path_effect_of_type(SPLPEItem *lpeitem, int type)
{
    std::list<Inkscape::LivePathEffect::LPEObjectReference *>::iterator i;
    for (i = lpeitem->path_effect_list->begin(); i != lpeitem->path_effect_list->end(); ++i) {
        Inkscape::LivePathEffect::Effect* lpe = (*i)->lpeobject->get_lpe();
        if (lpe && (lpe->effectType() == type)) {
            return lpe;
        }
    }
    return NULL;
}

/* Return false if the item is not a path or already has a shape applied */
bool sp_lpe_item_can_accept_freehand_shape(SPLPEItem *lpeitem)
{
    if (!SP_IS_PATH(lpeitem))
        return false;

    if (sp_lpe_item_has_path_effect_of_type(lpeitem, Inkscape::LivePathEffect::FREEHAND_SHAPE))
        return false;

    return true;
}

void sp_lpe_item_edit_next_param_oncanvas(SPLPEItem *lpeitem, SPDesktop *dt)
{
    Inkscape::LivePathEffect::LPEObjectReference *lperef = sp_lpe_item_get_current_lpereference(lpeitem);
    if (lperef && lperef->lpeobject && lperef->lpeobject->get_lpe()) {
        lperef->lpeobject->get_lpe()->editNextParamOncanvas(SP_ITEM(lpeitem), dt);
    }
}

static void
sp_lpe_item_child_added (SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *ref)
{
    if (((SPObjectClass *) (parent_class))->child_added)
        (* ((SPObjectClass *) (parent_class))->child_added) (object, child, ref);

    if (SP_IS_LPE_ITEM(object) && sp_lpe_item_has_path_effect_recursive(SP_LPE_ITEM(object))) {
        SPObject *ochild = sp_object_get_child_by_repr(object, child);
        if ( ochild && SP_IS_LPE_ITEM(ochild) ) {
            sp_lpe_item_create_original_path_recursive(SP_LPE_ITEM(ochild));
        }
    }
}

static void
sp_lpe_item_remove_child (SPObject * object, Inkscape::XML::Node * child)
{
    if (SP_IS_LPE_ITEM(object) && sp_lpe_item_has_path_effect_recursive(SP_LPE_ITEM(object))) {
        SPObject *ochild = sp_object_get_child_by_repr(object, child);
        if ( ochild && SP_IS_LPE_ITEM(ochild) ) {
            sp_lpe_item_cleanup_original_path_recursive(SP_LPE_ITEM(ochild));
        }
    }

    if (((SPObjectClass *) (parent_class))->remove_child)
        (* ((SPObjectClass *) (parent_class))->remove_child) (object, child);
}

static std::string patheffectlist_write_svg(PathEffectList const & list)
{
    HRefList hreflist;
    for (PathEffectList::const_iterator it = list.begin(); it != list.end(); ++it)
    {
        hreflist.push_back( std::string((*it)->lpeobject_href) );
    }
    return hreflist_write_svg(hreflist);
}

/**
 *  THE function that should be used to generate any patheffectlist string.
 * one of the methods to change the effect list:
 *  - create temporary href list
 *  - populate the templist with the effects from the old list that you want to have and their order
 *  - call this function with temp list as param
 */
static std::string hreflist_write_svg(HRefList const & list)
{
    std::string r;
    bool semicolon_first = false;
    for (HRefList::const_iterator it = list.begin(); it != list.end(); ++it)
    {
        if (semicolon_first) {
            r += ';';
        }
        semicolon_first = true;

        r += (*it);
    }
    return r;
}

// Return a copy of the effect list
PathEffectList sp_lpe_item_get_effect_list(SPLPEItem *lpeitem)
{
    return *lpeitem->path_effect_list;
}

Inkscape::LivePathEffect::LPEObjectReference* sp_lpe_item_get_current_lpereference(SPLPEItem *lpeitem)
{
    if (!lpeitem->current_path_effect && !lpeitem->path_effect_list->empty()) {
        sp_lpe_item_set_current_path_effect(lpeitem, lpeitem->path_effect_list->back());
    }

    return lpeitem->current_path_effect;
}

Inkscape::LivePathEffect::Effect* sp_lpe_item_get_current_lpe(SPLPEItem *lpeitem)
{
    Inkscape::LivePathEffect::LPEObjectReference* lperef = sp_lpe_item_get_current_lpereference(lpeitem);

    if (lperef && lperef->lpeobject)
        return lperef->lpeobject->get_lpe();
    else
        return NULL;
}

bool sp_lpe_item_set_current_path_effect(SPLPEItem *lpeitem, Inkscape::LivePathEffect::LPEObjectReference* lperef)
{
    for (PathEffectList::iterator it = lpeitem->path_effect_list->begin(); it != lpeitem->path_effect_list->end(); it++) {
        if ((*it)->lpeobject_repr == lperef->lpeobject_repr) {
            lpeitem->current_path_effect = (*it);  // current_path_effect should always be a pointer from the path_effect_list !
            return true;
        }
    }

    return false;
}

/**
 * Writes a new "inkscape:path-effect" string to xml, where the old_lpeobjects are substituted by the new ones.
 *  Note that this method messes up the item's \c PathEffectList.
 */
void SPLPEItem::replacePathEffects( std::vector<LivePathEffectObject const *> const old_lpeobjs,
                                    std::vector<LivePathEffectObject const *> const new_lpeobjs )
{
    HRefList hreflist;
    for (PathEffectList::const_iterator it = this->path_effect_list->begin(); it != this->path_effect_list->end(); ++it)
    {
        LivePathEffectObject const * current_lpeobj = (*it)->lpeobject;
        std::vector<LivePathEffectObject const *>::const_iterator found_it(std::find(old_lpeobjs.begin(), old_lpeobjs.end(), current_lpeobj));
        if ( found_it != old_lpeobjs.end() ) {
            std::vector<LivePathEffectObject const *>::difference_type found_index = std::distance (old_lpeobjs.begin(), found_it);
            const gchar * repr_id = SP_OBJECT_REPR(new_lpeobjs[found_index])->attribute("id");
            gchar *hrefstr = g_strdup_printf("#%s", repr_id);
            hreflist.push_back( std::string(hrefstr) );
            g_free(hrefstr);
        }
        else {
            hreflist.push_back( std::string((*it)->lpeobject_href) );
        }
    }
    std::string r = hreflist_write_svg(hreflist);
    SP_OBJECT_REPR(this)->setAttribute("inkscape:path-effect", r.c_str());
}

/**
 *  Check all effects in the stack if they are used by other items, and fork them if so.
 *  It is not recommended to fork the effects by yourself calling LivePathEffectObject::fork_private_if_necessary,
 *  use this method instead.
 *  Returns true if one or more effects were forked; returns false if nothing was done.
 */
bool sp_lpe_item_fork_path_effects_if_necessary(SPLPEItem *lpeitem, unsigned int nr_of_allowed_users)
{
    bool forked = false;

    if ( sp_lpe_item_has_path_effect(lpeitem) ) {
        // If one of the path effects is used by 2 or more items, fork it
        // so that each object has its own independent copy of the effect.
        // Note: replacing path effects messes up the path effect list

        // Clones of the LPEItem will increase the refcount of the lpeobjects.
        // Therefore, nr_of_allowed_users should be increased with the number of clones (i.e. refs to the lpeitem)
        nr_of_allowed_users += SP_OBJECT(lpeitem)->hrefcount;

        std::vector<LivePathEffectObject const *> old_lpeobjs, new_lpeobjs;
        PathEffectList effect_list =  sp_lpe_item_get_effect_list(lpeitem);
        for (PathEffectList::iterator it = effect_list.begin(); it != effect_list.end(); it++)
        {
            LivePathEffectObject *lpeobj = (*it)->lpeobject;
            if (lpeobj) {
                LivePathEffectObject *forked_lpeobj = lpeobj->fork_private_if_necessary(nr_of_allowed_users);
                if (forked_lpeobj != lpeobj) {
                    forked = true;
                    old_lpeobjs.push_back(lpeobj);
                    new_lpeobjs.push_back(forked_lpeobj);
                }
            }
        }

        if (forked) {
            lpeitem->replacePathEffects(old_lpeobjs, new_lpeobjs);
        }
    }

    return forked;
}

// Enable or disable the path effects of the item.
// The counter allows nested calls
static void sp_lpe_item_enable_path_effects(SPLPEItem *lpeitem, bool enable)
{
    if (enable) {
        lpeitem->path_effects_enabled++;
    }
    else {
        lpeitem->path_effects_enabled--;
    }
}

// Are the path effects enabled on this item ?
bool sp_lpe_item_path_effects_enabled(SPLPEItem *lpeitem)
{
    return lpeitem->path_effects_enabled > 0;
}

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
