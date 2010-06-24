#ifndef __SIMPLELISTVIEW_H__
#define __SIMPLELISTVIEW_H__

#include <deque>

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtktooltips.h>
#include <gtk/gtkliststore.h>

G_BEGIN_DECLS

#define SIMPLELISTVIEW_TYPE			(simplelistview_get_type())
#define SIMPLELISTVIEW(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), SIMPLELISTVIEW_TYPE, SimpleListView))
#define SIMPLELISTVIEW_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), SIMPLELISTVIEW_TYPE, SimpleListViewClass))
#define IS_SIMPLELISTVIEW(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), SIMPLELISTVIEW_TYPE))
#define IS_SIMPLELISTVIEW_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), SIMPLELISTVIEW_TYPE))

typedef struct _SimpleListView SimpleListView;
typedef struct _SimpleListViewClass SimpleListViewClass;


// Define your options like so:
// SimpleListViewOptions opts;
// opts.Add("Key1 ",_("Displayed text 1"),_("Optional tooltip"),optional_GdkPixBuf,optional_userdata);
// opts.Add("Key2 ",_("Displayed text 2"),_("Optional tooltip"));
// opts.Add("Other",_("Other..."),_("Optional tooltip"));	// Allow "other" to be triggered even if it's already selected...
//
// The options list will be copied, so can be safely constructed in local storage.
// (Any pixbuf, if given, will be referenced, not copied).


class SimpleListViewOption;
class SimpleListViewOptions : public std::deque<SimpleListViewOption *>
{
	public:
	SimpleListViewOptions();
	SimpleListViewOptions(SimpleListViewOptions &other);
	~SimpleListViewOptions();
	SimpleListViewOption *Add(const char *key,const char *displayname,const char *tooltip=NULL,
		GdkPixbuf *pixbuf=NULL, void *userdata=NULL);
	void SetIcon(int idx,GdkPixbuf *icon);
	void Clear();
	protected:
	friend class SimpleListViewOption;
};


class SimpleListViewOption
{
	public:
	SimpleListViewOption(const char *key,const char *displayname,const char *tooltip=NULL,
		GdkPixbuf *icon=NULL,void *userdata=NULL);
	SimpleListViewOption(SimpleListViewOption &other);
	~SimpleListViewOption();
	void SetIcon(GdkPixbuf *pixbuf);
	char *key;
	char *displayname;
	char *tooltip;
	GdkPixbuf *icon;
	void *userdata;
	protected:
};


struct _SimpleListView
{
	GtkHBox box;
	GtkTooltips *tips;
	GtkListStore *liststore;
	GtkWidget *treeview;
	GList *imagelist;
	SimpleListViewOptions *opts;
};


struct _SimpleListViewClass
{
	GtkHBoxClass parent_class;

	void (*changed)(SimpleListView *listview);
};

GType simplelistview_get_type (void);

GtkWidget* simplelistview_new(SimpleListViewOptions *opts);

SimpleListViewOption *simplelistview_get(SimpleListView *c);
//bool simplelistview_set(SimpleListView *c,const char *key);
int simplelistview_get_index(SimpleListView *c);
void simplelistview_set_index(SimpleListView *c,int index);

void simplelistview_set_opts(SimpleListView *c,SimpleListViewOptions *opts);

G_END_DECLS

#endif /* __SIMPLELISTVIEW_H__ */
