#ifndef __REMOVE_LAST_H__
#define __REMOVE_LAST_H__

#include <algorithm>
#include <vector>
#include <glib.h>

template<class T>
inline void remove_last(std::vector<T> &seq, T const &elem)
{
    using std::vector;

    typename vector<T>::reverse_iterator i(find(seq.rbegin(), seq.rend(), elem));
    g_assert( i != seq.rend() );
    //typename vector<T>::iterator ii(&*i);
    //seq.erase(ii);
	seq.erase(i.base());
}


#endif /* !__REMOVE_LAST_H__ */


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
