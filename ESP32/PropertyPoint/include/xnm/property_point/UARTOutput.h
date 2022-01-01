/**
 * @file UARTOutput.h
 * @author Neira David Bailey (davidbailey.2889@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-12-07
 * 
 */

#include "BaseHandler.h"
#include "BaseOutput.h"
#include "BaseProperty.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace XNM {
namespace PropertyPoint {

class UARTOutput : public BaseOutput {
private:
	static void _call_line_task(void *args);
	void line_task();

	TaskHandle_t line_task_handle;

protected:
	void send_upd_json(const cJSON * item, BaseProperty &prop);

public:
	UARTOutput(Handler &handler);

	/**
	 * @brief Start the UART task
	 * 
	 * @details This function will start the UART Output.
	 * 	This will enable a internal task that will loop and
	 * 	read in any data from UART. Note that this means
	 * 	that no other internal systems shall read in
	 * 	data from UART!
	 * 
	 * @todo Add a separate function to read in a line
	 */
	void init();
};

}
}