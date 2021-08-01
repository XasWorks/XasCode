


#define XNM_PROP_STATE_LIST_ARGS(wrap) \
	wrap(ERR), \
	wrap(OK)


namespace XNM {
namespace PropertyPoint {

#define XNM_PROP_STATE_LIST_NOWRAP(text) text
#define XNM_PROP_STATE_LIST_STRWRAP(text) #text

enum state_t {
	XNM_PROP_STATE_LIST_ARGS(XNM_PROP_STATE_LIST_NOWRAP)
};

__attribute__((used))
static const char* state_names[] = {
	XNM_PROP_STATE_LIST_ARGS(XNM_PROP_STATE_LIST_STRWRAP)
};

}
}