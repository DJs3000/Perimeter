#pragma once

#ifndef _KNOWLEDGE_HPP
#define _KNOWLEDGE_HPP

#include "SharedPointer.hpp"
#include "World.hpp"
#include <set>

class Knowledge : public tx3d::SharedPointer {
	public:
		Knowledge();
		Knowledge(const Knowledge& origin);
		~Knowledge() override;

		void addKnowledge(const Knowledge& anotherKnowledge);
		void worldVisited(World* world);
		
		bool knowAboutWorld(World* world) const {
			return knownWorlds.find(world) != knownWorlds.end();
		}
		const std::vector<World*>& getPath() const {
			return path;
		}
	protected:
		std::set<World*> knownWorlds;
		std::vector<World*> path;
};



#endif //_KNOWLEDGE_HPP
