#ifndef fooflxserverhfoo
#define fooflxserverhfoo

typedef struct _flxEntry flxEntry;
typedef struct _flxResponseJob flxResponseJob;

#include "flx.h"
#include "iface.h"
#include "prioq.h"
#include "llist.h"
#include "timeeventq.h"

struct _flxEntry {
    flxRecord *record;
    gint id;
    gint interface;
    guchar protocol;

    gboolean unique;

    FLX_LLIST_FIELDS(flxEntry, entry);
    FLX_LLIST_FIELDS(flxEntry, by_name);
    FLX_LLIST_FIELDS(flxEntry, by_id);
};

struct _flxResponseJob {
    flxTimeEvent *time_event;
    flxRecord *record;
    FLX_LLIST_FIELDS(flxResponseJob, response);
};

struct _flxServer {
    GMainContext *context;
    flxInterfaceMonitor *monitor;

    gint current_id;
    
    GHashTable *rrset_by_id;
    GHashTable *rrset_by_name;

    FLX_LLIST_HEAD(flxEntry, entries);

    flxTimeEventQueue *time_event_queue;
    
    gchar *hostname;
};


#endif
