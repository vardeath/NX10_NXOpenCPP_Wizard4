#include "my_menu.hpp"

extern "C" int ufusr_ask_unload()
{
	return (int)NXOpen::Session::LibraryUnloadOptionImmediately;
}
//Перегрузка операторов сравнения для NXString.
bool operator==(NXString& const val1, NXString& const val2) {
	string leftval = val1.GetText();
	string rightval = val2.GetText();
	return (leftval == rightval);
};
bool operator!=(NXString& val1, NXString& val2) {
	string leftval = val1.GetText();
	string rightval = val2.GetText();
	return !(leftval == rightval);
};
//Получаем массив всех обьектов в папке NC_PROGRAM.

void extractor(std::vector<CAM::CAMObject *>& x, CAM::NCGroup *& y, Part *& workPart) {
	std::vector<CAM::CAMObject *> temp = y->GetMembers();
	if (temp.size() > 0) {
		for (int i = 0; i < temp.size(); ++i) {
			bool isgroup = workPart->CAMSetup()->IsGroup(temp[i]);
			if (isgroup) {
				NXString gr_name = temp[i]->Name();
				CAM::NCGroup *temp_gr(dynamic_cast<NXOpen::CAM::NCGroup *>(workPart->CAMSetup()->CAMGroupCollection()->FindObject(gr_name)));
				extractor(x, temp_gr, workPart);
			}
			x.push_back(temp[i]);
		}
	}
}

//Проверка предлагаемого имени на совпадение.

bool comparator(std::vector<CAM::CAMObject *>& array, NXString name) {
	for (int i = 0; i < array.size(); ++i) {
		NXString obj_name = array[i]->Name();
		if (obj_name == name) return true;
	}
	return false;
}

//Разделяем введенное имя на 3 части, чтобы получить часть имени для нумерации.

void name_apart(string& got_name, string& part_name_1, string& part_name_2, string& part_name_3, char& change_symb) {

	bool used_change_symb = false;
	char char_instead = '0'; //Символ, подменяющий символ замены.

	for (int i = 0; i < got_name.size(); ++i) {
		if (got_name[i] != change_symb && !used_change_symb) {
			part_name_1 += got_name[i];
			continue;
		}
		if (got_name[i] == change_symb && !used_change_symb) {
			used_change_symb = true;
			got_name[i] = char_instead;
			part_name_2 += got_name[i];
			continue;
		}
		if (got_name[i] == change_symb && used_change_symb) {
			got_name[i] = char_instead;
			part_name_2 += got_name[i];
			continue;
		}
		if (got_name[i] != change_symb && used_change_symb) {
			part_name_3 += got_name[i];
			continue;
		}
	}
}

//Перевод строки в верхний регистр.

NXString to_up_register(NXString path_name) {
	string name_to_up = path_name.GetText();
	string upped_name;
	for (int i = 0; i < name_to_up.size(); ++i) {
		upped_name += toupper(name_to_up[i]);
	}
	return upped_name;
}

//Нумерация - предотвращает повторы в названиях и ошибки переименования.
void renumerator_function(string MAIN_FOLDER_NAME, NXString PATH_NAME, vector<CAM::CAMObject *>& all_parts, vector<CAM::CAMObject *>&  selected_group,
	CAM::NCGroup*& NC_tagged_object, int iterator, int iter_start_value, bool toggles_val,  bool change_state, char change_symb)
{
	string name_to_str = PATH_NAME.GetText();
	string part_name_1 = "";
	string part_name_2 = "";
	string part_name_3 = "";
	name_apart(name_to_str, part_name_1, part_name_2, part_name_3, change_symb);

	int max_iterations = 2; //Число запусков функции переименования.
	NXString object_name = NC_tagged_object->Name(); //Получаем имя выделенного обьекта.
	string object_name_str = object_name.GetText();

	for (int k = 0; k < max_iterations; ++k)
	{
		NXString path_name = "default";
		int path_num = iter_start_value;

		for (int i = 0; i < selected_group.size(); ++i)
		{
			if (MAIN_FOLDER_NAME == object_name_str && i == 0 && !toggles_val) continue;
			while (true) {
				NXString t_itera = std::to_string(path_num);
				if (!change_state) path_name = (k == 0) ? t_itera : PATH_NAME + "_" + t_itera; //Первый запуск - переименование итератором, второй - переименование согласно имени.
				else 
				{
					if (k == 0) path_name = t_itera;
					else 
					{
						string it_to_string = t_itera.GetText();
						if (it_to_string.size() >= part_name_2.size()) part_name_2 = it_to_string;
						else 
						{
							int iter_size = it_to_string.size();
							int p_name2_size = part_name_2.size();
							for (int i = iter_size; i >= 0; --i)
							{
								part_name_2[p_name2_size] = it_to_string[iter_size];
								--iter_size;
								--p_name2_size;
							}
						}
						path_name = part_name_1 + part_name_2 + part_name_3;
					}
				}
				NXString up_name = to_up_register(path_name);
				if (comparator(all_parts, up_name)) {
					path_num += iterator; //Если есть повтор имени, увеличиваем итератор, чтобы избежать ошибки переименования.
				} else break;
			}
			selected_group[i]->SetName(path_name);
			path_num += iterator;
		}
	}
}

void get_cam_group_to_num(Part *workPart ,vector<CAM::CAMObject *>&  selected_group, vector<CAM::CAMObject *>& under_group) {
	for (int i = 0; i < selected_group.size(); ++i)
	{
		if (workPart->CAMSetup()->IsGroup(selected_group[i]))
		{
			NXString gr_name = selected_group[i]->Name();
			CAM::NCGroup *Temp(dynamic_cast<NXOpen::CAM::NCGroup *>(workPart->CAMSetup()->CAMGroupCollection()->FindObject(gr_name)));
			std::vector<CAM::CAMObject *> Temp_group = Temp->GetMembers();
			for (int k = 0; k < Temp_group.size(); ++k)
			{
				under_group.push_back(Temp_group[k]);
			}
		}
	}
	if (under_group.size() == 0) { throw exception("Нет каталогов к нумерации"); }
}

//Переименование всех обьектов в выделенной группе.
void renumeration_by_toggle_state(Part *workPart, string main_name, vector<CAM::CAMObject *>& all_parts, CAM::NCGroup*& NC_tagged_object,
	NXString path_name, int iterator, int iter_start_value, bool toggles_val, bool change_state, char change_symb)
{
	std::vector<CAM::CAMObject *>  selected_group = NC_tagged_object->GetMembers();

	if (!toggles_val) {
		renumerator_function(main_name, path_name, all_parts, selected_group, NC_tagged_object, iterator, iter_start_value, toggles_val, change_state, change_symb);
	}
	else {
		std::vector<CAM::CAMObject *> under_group;
		get_cam_group_to_num(workPart, selected_group, under_group);
		renumerator_function(main_name, path_name, all_parts, under_group, NC_tagged_object, iterator, iter_start_value, toggles_val, change_state, change_symb);
	}
}

extern "C" void ufusr(char *param, int *retCode, int paramLen)
{
	try
	{
		//Список инициализации.
		UF_initialize();
		Session *theSession = Session::GetSession();
		Part *workPart(theSession->Parts()->Work());
		Part *displayPart(theSession->Parts()->Display());
		UI *theUI = UI::GetUI();

		string NC_NAME = "NC_PROGRAM";
		CAM::NCGroup *nCProgram(dynamic_cast<NXOpen::CAM::NCGroup *>(workPart->CAMSetup()->CAMGroupCollection()->FindObject(NC_NAME)));
		std::vector<CAM::CAMObject *> all_parts;
		extractor(all_parts, nCProgram, workPart);

		if (CAM::CAMObject * sel_obj = dynamic_cast<NXOpen::CAM::NCGroup *>(theUI->SelectionManager()->GetSelectedTaggedObject(0))) {
			CAM::NCGroup *verif(dynamic_cast<NXOpen::CAM::NCGroup *>(theUI->SelectionManager()->GetSelectedTaggedObject(0)));
			std::vector<CAM::CAMObject *> verif_members = verif->GetMembers();
			if (verif_members.size() < 1 ) { throw exception("Пустая папка для нумерации"); }
		}
		else { throw exception("Не выбрана папка для нумерации"); }
	

		if (theUI->SelectionManager()->GetSelectedTaggedObject(0) == 0) { throw exception("Не выбрана папка для нумерации"); }
		else
		{
			CAM::NCGroup *NC_tagged_object = dynamic_cast<NXOpen::CAM::NCGroup *>(theUI->SelectionManager()->GetSelectedTaggedObject(0));

			my_menu new_menu; // Создание и вызов диалогового меню.
			new_menu.Show();

			if (new_menu.cancel_return()) {}
			else
			{
				NXString got_name = new_menu.p_name_return();
				int got_path_iter = new_menu.p_iter_return();
				int got_start_path_value = new_menu.p_start_return();
				bool toggles_val = new_menu.toggle_state();
				char change_symb = new_menu.to_change(); //символ, который подменяется нумерацией.
				bool change_state = new_menu.sym_state();

				if (NC_tagged_object != 0) 
				{ //Нумерация по типу состояния переключателя.
					renumeration_by_toggle_state(workPart, NC_NAME, all_parts, NC_tagged_object, got_name, got_path_iter,
						got_start_path_value, toggles_val, change_state, change_symb);
				}
			}
		}
	}
	catch (const NXException& e1)
	{
		UI::GetUI()->NXMessageBox()->Show("NXException", NXOpen::NXMessageBox::DialogTypeError, e1.Message());
	}
	catch (const exception& e2)
	{
		UI::GetUI()->NXMessageBox()->Show("Exception", NXOpen::NXMessageBox::DialogTypeError, e2.what());
	}
	catch (...)
	{
		UI::GetUI()->NXMessageBox()->Show("Exception", NXOpen::NXMessageBox::DialogTypeError, "Unknown Exception.");
	}
}