#ifndef TLD_DEVICE_H
#define TLD_DEVICE_H

#include "GLEANKernel/Device_base.h"
#include "GLEANKernel/Smart_Pointer.h"
#include "GLEANKernel/Widgets.h"
#include <map>

class TLD_device;
// a simple device that puts Button widgets on the simulated display
// the color can be read and changed, whereupon the display changes.


class TLD_device : public Device_base {
public:
	TLD_device(const std::string& id, Output_tee& ot);

	virtual void initialize();
	virtual void handle_Start_event();
	virtual void handle_Point_event(const Symbol& target_name);
	virtual void handle_Click_event(const Symbol& button_name);
	virtual void handle_Delay_event(const Symbol& type, const Symbol& datum, 
		const Symbol& object_name, const Symbol& property_name, const Symbol& property_value);
private:
	Smart_Pointer<Screen_widget> screen_ptr;
	// a container of pointers to buttons
	typedef std::map<Symbol, Smart_Pointer<Button_widget> > buttons_t;
	buttons_t buttons;
	Smart_Pointer<Cursor_widget> cursor_ptr;
	
	
	Smart_Pointer<Object_widget> blip_ptr;	// at most one Blip can exist at a time
	int blip_counter;	// counts the number of time a blip was clicked on		

	Symbol current_pointed_to_object_name;	// holds name of current button pointed to

	void create_initial_display();
	void create_button(TLD_device * dev_ptr, const Symbol& name, GU::Point location, const Symbol& label, bool state);
	
	void clear_display();
	void output_display() const;

};

#endif
