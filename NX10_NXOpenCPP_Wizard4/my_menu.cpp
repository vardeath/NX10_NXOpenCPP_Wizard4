//------------------------------------------------------------------------------
//These includes are needed for the following template code
//------------------------------------------------------------------------------
#include "my_menu.hpp"
using namespace NXOpen;
using namespace NXOpen::BlockStyler;
//------------------------------------------------------------------------------
// Initialize static variables
//------------------------------------------------------------------------------
Session *(my_menu::theSession) = NULL;
UI *(my_menu::theUI) = NULL;
Part *(my_menu::workpart) = NULL;
Part *(my_menu::displayPart) = NULL;
//------------------------------------------------------------------------------
// Constructor for NX Styler class 
//------------------------------------------------------------------------------
my_menu::my_menu()
{
	try
	{
		// Initialize the NX Open C++ API environment
		my_menu::theSession = NXOpen::Session::GetSession();
		my_menu::theUI = UI::GetUI();
		my_menu::workpart = theSession->Parts()->Work();
		my_menu::displayPart = theSession->Parts()->Display();
		theDlxFileName = "Num_menu.dlx";
		theDialog = my_menu::theUI->CreateDialog(theDlxFileName);
		// Registration of callback functions
		theDialog->AddOkHandler(make_callback(this, &my_menu::ok_cb));
		theDialog->AddUpdateHandler(make_callback(this, &my_menu::update_cb));
		theDialog->AddCancelHandler(make_callback(this, &my_menu::cancel_cb));
		theDialog->AddInitializeHandler(make_callback(this, &my_menu::initialize_cb));
		theDialog->AddDialogShownHandler(make_callback(this, &my_menu::dialogShown_cb));
	}
	catch (exception& ex)
	{
		//---- Enter your exception handling code here -----
		//throw 1;
		my_menu::theUI->NXMessageBox()->Show("Block Styler", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
}
void my_menu::set_values(NXString p_name, NXString p_iter, NXString p_start, bool p_toggle, char sym, bool state) 
{
	string temp_iter = p_iter.GetText();
	int temp_iterator = std::stoi(temp_iter);

	string temp_start = p_start.GetText();
	int temp_starter = std::stoi(temp_start);

	pr_name = p_name;
	pr_iter = temp_iterator;
	start_path_value = temp_starter;
	toggle_val = p_toggle;
	canceled = false;
	sym_to_change = sym;
	state_by_sym = state;
}
//------------------------------------------------------------------------------
// Destructor for NX Styler class
//------------------------------------------------------------------------------
my_menu::~my_menu()
{
	if (theDialog != NULL)
	{
		delete theDialog;
		theDialog = NULL;
	}
}
//------------------------------------------------------------------------------
int my_menu::Show()
{
	try
	{
		theDialog->Show();
	}
	catch (exception& ex)
	{
		//---- Enter your exception handling code here -----
		my_menu::theUI->NXMessageBox()->Show("Block Styler", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
	return 0;
}
//------------------------------------------------------------------------------
//Callback Name: initialize_cb
//------------------------------------------------------------------------------
void my_menu::initialize_cb()
{
	try
	{
		string0 = dynamic_cast<NXOpen::BlockStyler::StringBlock*>(theDialog->TopBlock()->FindBlock("string0"));
		string01 = dynamic_cast<NXOpen::BlockStyler::StringBlock*>(theDialog->TopBlock()->FindBlock("string01"));
		string02 = dynamic_cast<NXOpen::BlockStyler::StringBlock*>(theDialog->TopBlock()->FindBlock("string02"));
		toggle0 = dynamic_cast<NXOpen::BlockStyler::Toggle*>(theDialog->TopBlock()->FindBlock("toggle0"));
		//------------------------------------------------------------------------------
		//Registration of StringBlock specific callbacks
		//------------------------------------------------------------------------------
		//string0->SetKeystrokeCallback(make_callback(this, &my_menu::KeystrokeCallback));

		//------------------------------------------------------------------------------
	}
	catch (exception& ex)
	{
		//---- Enter your exception handling code here -----
		my_menu::theUI->NXMessageBox()->Show("Block Styler", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
}

//------------------------------------------------------------------------------
//Callback Name: dialogShown_cb
//This callback is executed just before the dialog launch. Thus any value set 
//here will take precedence and dialog will be launched showing that value. 
//------------------------------------------------------------------------------
void my_menu::dialogShown_cb()
{
	try
	{
		//---- Enter your callback code here -----
	}
	catch (exception& ex)
	{
		//---- Enter your exception handling code here -----
		my_menu::theUI->NXMessageBox()->Show("Block Styler", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
}

//------------------------------------------------------------------------------
//Callback Name: update_cb
//------------------------------------------------------------------------------
int my_menu::update_cb(NXOpen::BlockStyler::UIBlock* block)
{
	try
	{
		if (block == string0)
		{
			//---------Enter your code here-----------
		}
		else if (block == string01)
		{
			//---------Enter your code here-----------
		}
	}
	catch (exception& ex)
	{
		//---- Enter your exception handling code here -----
		my_menu::theUI->NXMessageBox()->Show("Block Styler", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
	return 0;
}

//------------------------------------------------------------------------------
//Callback Name: ok_cb
//------------------------------------------------------------------------------
int my_menu::ok_cb()
{
	int errorCode = 0;
	try
	{
		char sym_to_change = 'x'; //Устанавливаем символ для подмены нумерацией.
		bool used_sym_ch = false; //Индикатор использования символа в введенном имени.
		int counter = 0; // Счетчик повторений групп символов подмены, если они идут не по порядку - с разрывом.
		bool total_using_changed_char = false;
		NXString p_name = string0->Value();
		string temp_p_name = p_name.GetText();
		if (temp_p_name.size() < 1) { throw exception("Пустое поле ввода"); }
		for (int i = 0; i < temp_p_name.size(); ++i) {
			if (temp_p_name[i] == '-' || temp_p_name[i] == '_' || temp_p_name[i] == '.') continue;
			if (isalpha(temp_p_name[i]) || isdigit(temp_p_name[i])) continue;
			else { throw exception("Допускаются к вводу: латинские буквы, числа, символ '-', символ '_', символ '.'"); }
		}
		//Проверка на количество блоков символов, введенных пользователем.
		for (int i = 0; i < temp_p_name.size(); ++i) {
			if (temp_p_name[i] == sym_to_change && !used_sym_ch)
			{
				used_sym_ch = true;
				++counter;
			}
			if (temp_p_name[i] != sym_to_change && used_sym_ch)
			{
				used_sym_ch = false;
			}
		}
		if (counter > 1) throw exception("Допускается только 1 блок символов для нумерации");
		if (counter == 1) total_using_changed_char = true;

		NXString p_iter = string01->Value();
		string temp_p_iter = p_iter.GetText();

		NXString p_start = string02->Value();
		string temp_p_start = p_start.GetText();

		bool p_toggle = toggle0->Value();
		if (temp_p_iter.size() < 1 || temp_p_start.size() < 1) { throw exception("Пустое поле ввода"); }

		for (int i = 0; i < temp_p_iter.size(); ++i) {
			if (!std::isdigit(temp_p_iter[i])){ throw exception("Инкремент должен быть числом"); }
		}
		for (int i = 0; i < temp_p_start.size(); ++i) {
			if (!std::isdigit(temp_p_start[i])) { throw exception("Аргумент должен быть числом"); }
		}

		int last_iter = std::stoi(temp_p_iter);
		int last_start = std::stoi(temp_p_start);

		if (last_iter < 1) { throw exception("Инкремент должен быть больше 0"); }
		if (last_start < 0) { throw exception("Начальное значение не может быть меньше 0"); }

		my_menu::set_values(p_name, p_iter, p_start, p_toggle, sym_to_change, total_using_changed_char);
	}
	catch (exception& ex)
	{

		//---- Enter your exception handling code here -----
		errorCode = 1;
		my_menu::theUI->NXMessageBox()->Show("Ошибка ввода данных", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
	return errorCode;
}

//------------------------------------------------------------------------------
//Callback Name: cancel_Cb
//------------------------------------------------------------------------------
int my_menu::cancel_cb()
{
	try
	{
		//return 1;
		//my_menu::theUI->NXMessageBox()->Show("Block Styler", NXOpen::NXMessageBox::DialogTypeInformation, "canceled");
	}
	catch (exception& ex)
	{
		//---- Enter your exception handling code here -----
		my_menu::theUI->NXMessageBox()->Show("Block Styler", NXOpen::NXMessageBox::DialogTypeError, ex.what());
	}
	return 0;
}
//------------------------------------------------------------------------------
PropertyList* my_menu::GetBlockProperties(const char *blockID)
{
	return theDialog->GetBlockProperties(blockID);
}
//------------------------------------------------------------------------------