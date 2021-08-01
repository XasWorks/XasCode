
#pragma once

#include "BaseProperty.h"

namespace XNM {
namespace PropertyPoint {

class JSONObjProperty : public BaseProperty {
private:
	cJSON * obj;

protected:
	void process_json_command(const cJSON * data);
	cJSON * get_current_state();


public:
	bool merge_json;

	JSONObjProperty(Handler &handler, const char *key);
	~JSONObjProperty();

	cJSON * fetch_key(const char *key);
	cJSON * operator[](const char *key);

	void delete_key(const char *key);

	double get_num(const char *key);
	void set_num(double value, const char *key);

	bool get_bool(const char *key);
	void set_bool(bool val, const char *key);

	void update_done();
};

}
}