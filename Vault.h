#include <cstdint>
#include <vector>
#include <any>
#include <exception>
#include <utility>
#include <memory>
#include <string>

namespace ForgECS
{
	using Entity = uint16_t;

	static constexpr size_t MAX_ENTITIES = 1000; //1000

	static constexpr size_t NULL_INDEX = static_cast<size_t>(-1);

	static constexpr size_t MAX_PAGE_SIZE = 1024; //1024

	static constexpr size_t MAX_PAGE_COUNT = (MAX_ENTITIES + MAX_PAGE_SIZE - 1) / MAX_PAGE_SIZE;

	class Vault
	{
	public:
		~Vault() {};

		//Creates a new entity - pass an optional title to track the entity later
		//You only need to give entities that you will directly interact with a title
		Entity Forge(std::string title = "")
		{
			Entity entity = entityCount; 
			entityCount++;

			entities.push_back(std::make_pair(entity, title));

			return entity;
		}

		//Destroys an entity - pass the entity to destroy
		void Dismantle(Entity entity)
		{
			//remove from entities list
			for (auto iter = entities.begin(); iter != entities.end(); ++iter)
			{
				if (iter->first == entity)
				{
					entities.erase(iter);

					//remove entities components
					for (auto& set : vault)
					{
						if (set->HasComponent(entity))
							set->DeleteComponent(entity);
					}

					return;
				}
			}

			throw std::exception();
		}

		//Adds a component to an entity - pass the entity and the component values
		template<typename T, typename... Args> bool AddComponent(Entity entity, Args&&... args)
		{
			for (auto& set : vault)
			{
				if (set->Type() == typeid(T))
				{
					// Archive the component in the existing sparse set
					T component(std::forward<Args>(args)...);
					set->ArchiveComponent(entity, std::make_any<T>(component));  // Store as std::any
					return true;
				}
			}

			// If type doesn't exist, create a new SparseSet for this component type
			auto sparseSet = std::make_unique<SparseSet<std::any>>(typeid(T), MAX_ENTITIES);
			T component(std::forward<Args>(args)...);
			sparseSet->ArchiveComponent(entity, std::make_any<T>(component));  // Store as std::any
			vault.push_back(std::move(sparseSet));

			return true;
		}

		//Retrieves a component from an entity - pass the entity and the component type
		template <typename T> T& GetComponent(Entity entity)
		{
			for (auto& set : vault)
			{
				if (set->Type() == typeid(T))
				{
					return std::any_cast<T&>(set->RetrieveComponent(entity));
				}
			}

			throw std::exception();
		}

		//Removes a component from an entity - pass the entity and the component type
		template <typename T> bool RemoveComponent(Entity entity)
		{
			for (auto& set : vault)
			{
				if (set->Type() == typeid(T))
				{
					set->DeleteComponent(entity);
		
					return true;
				}
			}
		
			return false;
		}

		//Retrieves all entities with this component type - pass the component type
		template <typename T> std::vector<Entity> GetAllByComponent()
		{
			std::vector<Entity> result;

			for (auto& set : vault)
			{
				if (set->Type() == typeid(T))
				{
					for (const auto& ent : entities)
					{
						if (set->HasComponent(ent.first))
							result.push_back(ent.first);
					}

					break;
				}
			}

			return result;
		}

		//Retrieves an entity by its title - pass the title given at Forge
		Entity GetByTitle(std::string title)
		{
			for (auto& ent : entities)
			{
				if (ent.second == title)
					return ent.first;
			}

			throw std::exception();
		}

	private:
		//This class creates and hold unique component type and their instances
		template <typename T> class SparseSet
		{
		public:
			SparseSet(const std::type_info& componentType, size_t maxEntities) : sparse(MAX_PAGE_COUNT), typeIndex(componentType)
			{
				dense.reserve(maxEntities);
				for (auto& page : sparse)
					page.resize(MAX_PAGE_SIZE, NULL_INDEX);
			}

			const std::type_info& Type() const { return typeIndex; }

			void ArchiveComponent(Entity id, T component)
			{
				dense.push_back(component);

				size_t pageIndex = id / MAX_PAGE_SIZE;
				size_t index = id % MAX_PAGE_SIZE;

				sparse[pageIndex][index] = dense.size() - 1;
			}

			T& RetrieveComponent(Entity id)
			{
				size_t pageIndex = id / MAX_PAGE_SIZE;
				size_t index = id % MAX_PAGE_SIZE;

				if (pageIndex >= sparse.size() || index >= sparse[pageIndex].size())
					throw std::exception();

				size_t denseIndex = sparse[pageIndex][index];

				if (denseIndex == NULL_INDEX)
					throw std::exception();

				return dense[denseIndex];
			}

			void DeleteComponent(Entity id)
			{
				if (dense.empty())
					throw std::exception();

				size_t pageIndex = id / MAX_PAGE_SIZE;
				size_t index = id % MAX_PAGE_SIZE;

				if (pageIndex >= sparse.size() || index >= sparse[pageIndex].size())
					throw std::exception();

				size_t denseIndex = sparse[pageIndex][index];

				if(denseIndex == NULL_INDEX)
					throw std::exception();

				if (dense.size() == 1)
				{
					sparse[pageIndex][index] = NULL_INDEX;
					dense.pop_back();
					return;
				}

				Entity backEntity = sparse[pageIndex][dense.size() - 1];  // Entity(dense.size() - 1); //sparse[dense.size() - 1];

				std::swap(dense[denseIndex], dense.back());

				sparse[pageIndex][index] = NULL_INDEX;
				sparse[backEntity / MAX_PAGE_SIZE][backEntity % MAX_PAGE_SIZE] = denseIndex;

				dense.pop_back();
			}

			bool HasComponent(Entity id)
			{
				size_t pageIndex = id / MAX_PAGE_SIZE;
				size_t index = id % MAX_PAGE_SIZE;

				if (pageIndex >= sparse.size() || index >= sparse[pageIndex].size())
					return false;

				return sparse[pageIndex][index] != NULL_INDEX;
			}

		private:
			//Sparse array - initialized size with preallocated storage
			std::vector<std::vector<size_t>> sparse;

			//Dense array - elements are packed leaving no empty elements
			std::vector<T> dense;

			//Variable so we know what type the unique component type is for this Sparce Set
			const std::type_info& typeIndex;
		};

		//still unsure about this
		template <typename Type> struct Type_Map
		{
			std::vector<std::pair<uint32_t, Type>> map;


		};
	

		//The vault contains all of the entities/components
		std::vector<std::unique_ptr<SparseSet<std::any>>> vault;
		//Entities keeps track of all the entities
		std::vector<std::pair<Entity, std::string>> entities;

		uint16_t entityCount = 0;
	};
};