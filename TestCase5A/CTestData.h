#ifndef CTESTDATA_H
#define CTESTDATA_H

#include"Model.h"
class CTestData{
private:
	Model *m_Model;

	void __testCulling(){
		Mesh *Cube1 = new Mesh(glm::vec3(0, 0, -2), 2.0);
		Mesh *Cube2 = new Mesh(glm::vec3(0, 0, -4), 1.0);
		Mesh *Cube3 = new Mesh(glm::vec3(1.7, 0, -2), 1.0);
		Mesh *Cube4 = new Mesh(glm::vec3(4, 0, -2), 1.0);
		Mesh *Cube5 = new Mesh(glm::vec3(1, 1, 1), 1.0);

		m_Model->meshes.push_back(Cube1);
		m_Model->meshes.push_back(Cube2);
		m_Model->meshes.push_back(Cube3);
		m_Model->meshes.push_back(Cube4);
		m_Model->meshes.push_back(Cube5);

	}
public:
	CTestData(){}
	CTestData(int vTestCase){
		m_Model = new Model();
		//
		//
		//
		//3->test the culling ,generate 5 cube and 1 is hide
		switch(vTestCase){
			case 3:{
				__testCulling();
			}
		}
	}

	Model getModel(){return *m_Model;}


};

#endif