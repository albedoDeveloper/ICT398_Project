#include "LevelLoader.h"
#include "GameObjectFactory.h"
#include <nlohmann/json.hpp>
#include <filesystem>
#include <iostream>
#if _DEBUG
#include "Transform.h"
#endif

using json = nlohmann::json;
namespace fs = std::filesystem;


LevelLoader::LevelLoader()
{
	objectList = GAMEOBJECT->GetObjectMap();
}

void LevelLoader::JsonFilepath()
{

	fs::path myPath = fs::current_path();

	fs::path newPath = (myPath /= "../Assets//SaveFiles");

	std::cout << newPath << std::endl;

	if (fs::create_directories(newPath))
	{
		std::cout << "SUCCESS" << std::endl;
	}

	if (!fs::is_directory("../Assets//SaveFiles") || !fs::exists("../Assets//SaveFiles"))
	{
		std::cout << "File doesn't exist" << std::endl;
	}

	//////end file pathing

}

void LevelLoader::ToJson(json &j, GameObject *g)
{

	j[g->getFactoryKey()]["key"] = g->getFactoryKey();

	//this transofrm stuff can be moved to the transform componenet instead

	g->GetTransform()->ToJson(j, g->getFactoryKey());

	/*j[g->getFactoryKey()]["Position"] =
	{
		{"x",g->GetTransform()->GetPosition().GetX()},
		{"y",g->GetTransform()->GetPosition().GetY()},
		{"z",g->GetTransform()->GetPosition().GetZ()},
	};

	j[g->getFactoryKey()]["Rotation"] =
	{
		{"x",g->GetTransform()->GetRotation().GetX()},
		{"y",g->GetTransform()->GetRotation().GetY()},
		{"z",g->GetTransform()->GetRotation().GetZ()},
		{"w",g->GetTransform()->GetRotation().GetW()},
	};

	j[g->getFactoryKey()]["Scale"] =
	{
		{"x",g->GetTransform()->GetRelativeScale().GetX()},
		{"y",g->GetTransform()->GetRelativeScale().GetY()},
		{"z",g->GetTransform()->GetRelativeScale().GetZ()},
	};*/

	//here is where we should get the componenet map and save to colliders, this will require each comp to have a tojson method


}

void LevelLoader::FromJson(json &j, GameObject *g)
{
	//debug to console
	/*std::cout << "load object" << j.at(g->getFactoryKey()).at("key") << std::endl;
	std::cout << " X = " << j.at(g->getFactoryKey()).at("Position").at("x") << std::endl;
	std::cout << " Y = " << j.at(g->getFactoryKey()).at("Position").at("y") << std::endl;
	std::cout << " Z = " << j.at(g->getFactoryKey()).at("Position").at("z") << std::endl;*/

	//NEED HERE instantiate the object (or at least in the load statement right before calling this method )

		//////ERROR HANDLING///////
	//////Need to figure out a way to check if the field in json is filled or not before calling from

	g->GetTransform()->FromJson(j, g->getFactoryKey());

	////move to position
	//if (j.at(g->getFactoryKey()).contains("Position"))
	//{
	//	g->GetTransform()->SetPosition(j.at(g->getFactoryKey()).at("Position").at("x"),
	//		j.at(g->getFactoryKey()).at("Position").at("y"),
	//		j.at(g->getFactoryKey()).at("Position").at("z"));
	//}
	//
	////set rotation, might need new keyword here i don't like declaring this temporary value
	//if (j.at(g->getFactoryKey()).contains("Rotation"))
	//{
	//	Quaternion q;

	//	q.SetX(j.at(g->getFactoryKey()).at("Rotation").at("x"));
	//	q.SetY(j.at(g->getFactoryKey()).at("Rotation").at("y"));
	//	q.SetZ(j.at(g->getFactoryKey()).at("Rotation").at("z"));
	//	q.SetW(j.at(g->getFactoryKey()).at("Rotation").at("w"));

	//	g->GetTransform()->SetRotation(q);
	//}
	//

	////change scale
	//if (j.at(g->getFactoryKey()).contains("Scale") )
	//{
	//	Vector3f v;
	//	v.SetX(j.at(g->getFactoryKey()).at("Scale").at("x"));
	//	v.SetY(j.at(g->getFactoryKey()).at("Scale").at("y"));
	//	v.SetZ(j.at(g->getFactoryKey()).at("Scale").at("z"));

	//	g->GetTransform()->SetRelativeScale(v);
	//}


	//here is where we add in the componenets from the json, each comp will need it's own fromJson method
}


void LevelLoader::LoadLevel()
{
	//Step 1 read filestream into json object
	std::ifstream ifs("../Assets/SaveFiles/tavern.json");
	json j = json::parse(ifs);

	//std::cout << j << std::endl;

	//Step 2 retrieve and populate map
	std::map<std::string, GameObject *>::iterator it;

	int i = 0;



	std::cout << "Loading level" << std::endl;



	//Step 3
	//This is the more robust method
	//We test for how many objects there are 
	int numOfObjects = j.size();
	std::cout << "NUM OF OBJECT == " << numOfObjects << std::endl;

	for (auto it : j.items())
	{
		//std::cout << "TEST " << j.at(el.key()).at("key") << std::endl;
		//std::cout << "TEST " << j.at(el.key()).at("Position").at("x") << std::endl;

		GAMEOBJECT->SpawnGameObject(j.at(it.key()).at("key"));

		//g->Load(j);
	}

	//this iterator only works if we already have all objects in GO factory
		//need a more robust method
	for (it = objectList->begin(); it != objectList->end(); it++)
	{
		//we read each gameobject from JSON
		it->second->Load(j);
		//FromJson(j, it->second);

	}
}

void LevelLoader::SaveLevel()
{
	// Step 1 make json
	json j;

	/// Step 2 File pathing and tests
	JsonFilepath();


	///Step 3 now we need to generate the JSON
	//we need to make this universally accessible
	std::ofstream o("../Assets/SaveFiles/tavern.json");


	//STEP 4 CHANGE
	GAMEOBJECT->Save(j);

	//Step 4 retrieve and populate Gameobject data
	//std::map<std::string, GameObject*>::iterator it;

	//int i = 0;

	//for (it = objectList->begin(); it != objectList->end(); it++)
	//{
	//	//console logs, can delete later but useful for now
	//	std::cout << "Object " << i << " Of " << objectList->size() << "\n" <<
	//		"	 has Key " << it->second->getFactoryKey() << "\n" <<
	//		"	 Is at position x=" << it->second->GetTransform()->GetPosition().GetX() << "\n" <<
	//		"	 Is at position y=" << it->second->GetTransform()->GetPosition().GetY() << "\n" <<
	//		"	 Is at position z=" << it->second->GetTransform()->GetPosition().GetZ() << "\n" <<
	//		": " <<
	//		"	 Is at rotation x = "<< it->second->GetTransform()->GetRotation().GetX() << "\n" <<
	//		"	 Is at rotation y = "<< it->second->GetTransform()->GetRotation().GetY() << "\n" <<
	//		"	 Is at rotation z = "<< it->second->GetTransform()->GetRotation().GetZ() << "\n" <<
	//		"	 Is at rotation w = "<< it->second->GetTransform()->GetRotation().GetW() << "\n" <<
	//		": "
	//		"	 has scale x=" << it->second->GetTransform()->GetRelativeScale().GetX() << "\n" <<
	//		"	 has scale y=" << it->second->GetTransform()->GetRelativeScale().GetY() << "\n" <<
	//		"	 has scale z=" << it->second->GetTransform()->GetRelativeScale().GetZ() << "\n" <<
	//		": " << std::endl;

	//	//Step 5 write each gameobject to JSON
	//	ToJson(j, it->second);

	//	i++;
	//}

	//Step 6 save json to file
	o << std::setw(4) << j << std::endl;

}
