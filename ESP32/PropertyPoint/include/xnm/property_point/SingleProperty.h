

#include "BaseProperty.h"

#include <xasin/neocontroller/Color.h>
#include <string>

namespace XNM {
namespace PropertyPoint {

template<class T>
class SingleProperty : public BaseProperty {
private:
	T value;

public:
	SingleProperty(Handler &handler, const char *key, const T &default_v) : BaseProperty(handler, key), value(default_v) {}

	cJSON * get_current_state() {
		return cJSON_CreateNull();
	};
	void process_json_command(const cJSON * data) {};


	T get_value() {
		return value;
	};
	void update_value(const T &ref) {
		if(ref == value)
			return;
		
		value = ref;
		poke_update();
	}
	void set_value(const T &ref) {
		if(readonly)
			return;
		
		update_value(ref);
	};

};

#define SinglePropertySpecialization(type) \
template<> \
cJSON * SingleProperty<type>::get_current_state(); \
template<> \
void SingleProperty<type>::process_json_command(const cJSON *);

SinglePropertySpecialization(double);
SinglePropertySpecialization(std::string);

SinglePropertySpecialization(Xasin::NeoController::Color);

}
}