#include "TLD_device.h"
#include "GLEANKernel/Device_exception.h"
#include "GLEANKernel/Glean_standard_symbols.h"
#include "GLEANKernel/Output_tee_globals.h"
#include "GLEANKernel/Utility_templates.h"	// for Delete_second
#include "GLEANKernel/Numeric_utilities.h"	// for random_int
#include "GLEANKernel/Symbol_utilities.h"	// for concatenate_to_Symbol
#include <iostream>
#include <iomanip>

using namespace std;

const Symbol Button1_c("Button1");
const Symbol Button2_c("Button2");
const Symbol Button3_c("Button3");
const Symbol Button4_c("Button4");
const Symbol Blip_c("Blip");


/*
This device is used to demonstrate some simple Top-Level Loop structures.

Demo1 produces a display with vertical layout of buttons, 
the second and fourth of which are red. When clicked on, they turn green.

A Blip appears at a random time after the start; if clicked-on it will reappear
at another random time for a total of three times, and then stop the simulation.
??? these may be out date!
*/


const GU::Size default_button_size(50, 20);

TLD_device::TLD_device(const std::string& id, Output_tee& ot) :
	Device_base(id, ot), blip_ptr(0), blip_counter(0)
{
	// If one object property refers to another object, GLEAN currently requires that 
	// the physical name be same as local names ...
	Widget::set_add_widget_type_property(true);  // e.g. so "Type Button" will be a property of an object
}

void TLD_device::clear_display()
{
	buttons.clear();
	screen_ptr = 0;
	blip_ptr = 0;
}

void TLD_device::initialize()
{
	Device_base::initialize();
	clear_display();
	
	current_pointed_to_object_name = Nil_c;
	// if  a blip was left up, get rid of it
	blip_ptr = 0;
	blip_counter = 0;
	// set up the initial display
	create_initial_display();
}

void TLD_device::create_button(TLD_device * dev_ptr, const Symbol& name, GU::Point location, const Symbol& label, bool state)
{
	Smart_Pointer<Button_widget> ptr = new Button_widget(dev_ptr, name, location, default_button_size, label, Red_c, Green_c, state);
	buttons[name] = ptr;
	screen_ptr->add_widget(ptr);
}

void TLD_device::create_initial_display()
{
	screen_ptr = new Screen_widget(this, Symbol("Screen"), GU::Point(0, 0), Widget::get_screen_pixel_size());
	screen_ptr->add_widget(cursor_ptr = new Cursor_widget(this, GU::Point(250, 250), GU::Size(20, 20)));
	create_button(this, Button1_c, GU::Point(500, 200), "Alpha", false);
	create_button(this, Button2_c, GU::Point(500, 300), "Beta ", true);
	create_button(this, Button3_c, GU::Point(500, 400), "Gamma", true);
	create_button(this, Button4_c, GU::Point(500, 500), "Delta", true);
	
	
	buttons[Button1_c]->set_property(Position_c, Top_c);
	buttons[Button1_c]->set_property(Above_c, Button2_c);
	buttons[Button1_c]->set_property(Below_c, Nil_c);

	buttons[Button2_c]->set_property(Position_c, "Middle");
	buttons[Button2_c]->set_property(Above_c, Button3_c);
	buttons[Button2_c]->set_property(Below_c, Button1_c);

	buttons[Button3_c]->set_property(Position_c, "Middle");
	buttons[Button3_c]->set_property(Above_c, Button4_c);
	buttons[Button3_c]->set_property(Below_c, Button2_c);

	buttons[Button4_c]->set_property(Position_c, Bottom_c);
	buttons[Button4_c]->set_property(Above_c, Nil_c);
	buttons[Button4_c]->set_property(Below_c, Button3_c);
}

// create the initial display when the simulation starts
void TLD_device::handle_Start_event()
{
	screen_ptr->present();
	
	// schedule the first blip appearance sometime between 2 sec and 3 secs from now
//	int delay = 2000 + random_int(3000);
	// schedule the first blip appearance sometime between 6 sec and 9 secs from now
	// give the button pressing time to get done
	int delay = 6000 + random_int(3000);
	schedule_delay_event(delay, "Blip_appear", "");			

	output_display();
}


// Output the state of the buttons and the cursor as a text display
void TLD_device::output_display() const
{
	device_out << "--------------------\n" << processor_info() << " Display: " << endl;
	for(buttons_t::const_iterator it = buttons.begin(); it != buttons.end(); ++it) {
		device_out << (it->second)->get_name() 
		<< ' ' << (it->second)->get_location() << ' ' << (it->second)->get_state() << endl;
		}
	if(blip_ptr) {
		device_out << blip_ptr->get_name() 
		<< ' ' << blip_ptr->get_location() << endl;
		}
	device_out << "Cursor pointing at " << current_pointed_to_object_name << endl;
	device_out << "--------------------" << endl;;
	
}

// If the user points to an object, remember what was pointed-to,
// and modify the display accordingly.
void TLD_device::handle_Point_event(const Symbol& target_name)
{
	if(Trace_out && get_trace())
		Trace_out << processor_info() << " Point_to: " << target_name << endl;
	if (target_name == "Absent" || target_name == Nil_c)
		Normal_out << processor_info() << " Point_to non-existent object: " << target_name << endl;
	current_pointed_to_object_name = target_name;
	Smart_Pointer<Widget> ptr = screen_ptr->get_widget_ptr(current_pointed_to_object_name);
	if(ptr) {
		GU::Point new_loc = ptr->get_location();
		cursor_ptr->set_location(new_loc);
		}
	
	set_visual_object_property(Cursor_name_c, Pointing_to_c, target_name);
	output_display();
}

// If it was a button, or the blip, that was clicked on, and it was red, change it to green.
void TLD_device::handle_Click_event(const Symbol& button_name)
{
	if(Trace_out && get_trace())
		Trace_out << processor_info() << " Click: " << button_name 
		<< " on " << current_pointed_to_object_name << endl;
	
//	if(blip_ptr && current_pointed_to_object_name == blip_ptr->get_name() && blip_ptr->get_property() == Red_c) {
	if(blip_ptr && current_pointed_to_object_name == blip_ptr->get_name()) {
		blip_ptr->set_property(Color_c, Green_c);
		// make the blip disappear a tenth second from now
		schedule_delay_event(100, "Blip_disappear", "");
		output_display();
		return;
		}
	else {
		buttons_t::iterator it = buttons.find(current_pointed_to_object_name);
		if(it == buttons.end())
			throw Device_exception(this, "Click-on unrecognized object");		
		Smart_Pointer<Button_widget> current_button_ptr = it->second;
		if(!(current_button_ptr->get_state()))
			throw Device_exception(this, "Click-on button that is off");		
		current_button_ptr->set_state(false);
		}
	output_display();
}


// the device delay event is either to make a blip appear, or disappear and either 
// schedule a new appearance or stop the simulation
void TLD_device::handle_Delay_event(const Symbol& type, const Symbol& datum, 
		const Symbol& object_name, const Symbol& property_name, const Symbol& property_value)
{
	if(Trace_out && get_trace())
		Trace_out << processor_info() << " Delay event: " 
		<< type << ' ' << datum << ' ' 
		<< object_name << ' ' << property_name << ' '  << property_value 
		<< endl;
	
	if(type == "Blip_appear") {
		ostringstream oss;
		oss << "Blip" << blip_counter;
		string blip_name = oss.str();
		blip_ptr = new Object_widget(this, Symbol(blip_name), GU::Point(240, 200 + 50 * blip_counter), GU::Size(20, 20));
		blip_ptr->set_property(Shape_c, Triangle_c);
		blip_ptr->set_property(Color_c, Red_c);
		screen_ptr->add_widget(blip_ptr);
		blip_ptr->present();
		}
	else if(type == "Blip_disappear") {
		if(!blip_ptr)
			throw Device_exception(this, "Event to disappear nonexisting Blip received");
		if(current_pointed_to_object_name == blip_ptr->get_name())
			current_pointed_to_object_name = Nil_c;
		screen_ptr->remove_widget(blip_ptr);
		blip_ptr = 0;
		blip_counter++;
		// stop the simulation if that was the third click-on
		if(blip_counter > 3) {
			stop_simulation();
			return;
			}
		// schedule a new appearance sometime between 1 sec and 2 secs from now
		int delay = 1000 + random_int(2000);
		schedule_delay_event(delay, "Blip_appear", "");			
		}
	else {
		throw Device_exception(this, "Unrecognized Delay event received");
		}
	
	output_display();
}
		
