#ifndef __SIMPLECOMBO_H__
#define __SIMPLECOMBO_H__


#include <glib.h>
#include <glib-object.h>
#include <gtk/gtkoptionmenu.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtktooltips.h>

G_BEGIN_DECLS

#define SIMPLECOMBO_TYPE			(simplecombo_get_type())
#define SIMPLECOMBO(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), SIMPLECOMBO_TYPE, SimpleCombo))
#define SIMPLECOMBO_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), SIMPLECOMBO_TYPE, SimpleComboClass))
#define IS_SIMPLECOMBO(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), SIMPLECOMBO_TYPE))
#define IS_SIMPLECOMBO_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), SIMPLECOMBO_TYPE))

typedef struct _SimpleCombo SimpleCombo;
typedef struct _SimpleComboClass SimpleComboClass;


// Define your options like so:
// SimpleComboOptions opts;
// opts.Add("Key1 ",_("Displayed text 1"),_("Optional tooltip"));
// opts.Add("Key2 ",_("Displayed text 2"),_("Optional tooltip"));
//
// The options list will be copied, so can be safely constructed in local storage.


class SimpleComboOption;
class SimpleComboOptions
{
	public:
	SimpleComboOptions();
	SimpleComboOptions(SimpleComboOptions &other);
	~SimpleComboOptions();
	SimpleComboOption *Add(const char *key,const char *displayname,const char *tooltip=NULL);
	SimpleComboOption *FirstOption();
	SimpleComboOption *operator[](int idx);
	protected:
	SimpleComboOption *firstopt;
	friend class SimpleComboOption;
};


class SimpleComboOption
{
	public:
	SimpleComboOption(SimpleComboOptions &header,const char *key,const char *displayname,const char *tooltip=NULL);
	SimpleComboOption(SimpleComboOptions &header,SimpleComboOption &other);
	~SimpleComboOption();
	SimpleComboOption *NextOption();
	SimpleComboOption *PrevOption();
	char *key;
	char *displayname;
	char *tooltip;
	protected:
	SimpleComboOptions &header;
	SimpleComboOption *prevopt,*nextopt;
};


// When declaring your array of SimpleComboOptions, you should mark the displayname members
// with N_() - the widget will call gettext() to translate them when building the combo.

//struct SimpleComboOption
//{
//	const char *option;
//	const char *displayname;
//};


struct _SimpleCombo
{
	GtkHBox box;
	GtkWidget *optionmenu;
	GtkWidget *menu;
	GtkTooltips *tips;
	SimpleComboOptions *opts;
	int previdx;
};


struct _SimpleComboClass
{
	GtkHBoxClass parent_class;

	void (*changed)(SimpleCombo *combo);
};

GType simplecombo_get_type (void);
GtkWidget* simplecombo_new (SimpleComboOptions &opts);
const char *simplecombo_get(SimpleCombo *c);
bool simplecombo_set(SimpleCombo *c,const char *key);
G_END_DECLS

#endif /* __SIMPLECOMBO_H__ */
