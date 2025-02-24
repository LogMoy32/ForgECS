#include "Vault.h"
#include <iostream>
#include <chrono>

struct Transform
{
    float x, y, z = 0;
};
struct Mesh
{
	std::string name = "None";
	int vertices = 0;
};

int main()
{
	// Start the timer
	auto start = std::chrono::high_resolution_clock::now();

	//Create storage vault for entities and components
    ForgECS::Vault vault;

	//Create an entity with optional title to retrieve it later
	auto player = vault.Forge("Player");
	auto enemy = vault.Forge("Enemy");
	auto bullet = vault.Forge();
	
	//Add components to entities with optional data
	vault.AddComponent<Transform>(player, 0, 0, 0);
	vault.AddComponent<Mesh>(player, "PlayerMesh", 100);
	vault.AddComponent<Transform>(enemy, 10, 10, 10);
	vault.AddComponent<Mesh>(enemy, "EnemyMesh", 200);
	vault.AddComponent<Transform>(bullet, 5, 5, 5);
	vault.AddComponent<Mesh>(bullet, "BulletMesh", 50);

	//Retrieve a component from an entity
	auto playerTransfrom = vault.GetComponent<Transform>(player);

	//does not use a bitfield for look-up yet
	std::vector<ForgECS::Entity> entities = vault.GetAllByComponent<Mesh>();

	//Retrieve an entity by its title given at Forge
	auto newPlayer = vault.GetByTitle("Player");

	//removed an entity and its components
	vault.Dismantle(bullet);

	//Remove specified component from given entity
	vault.RemoveComponent<Mesh>(player);

    // End the timer
    auto end = std::chrono::high_resolution_clock::now();

    // Calculate and print the elapsed time
    std::chrono::duration<double> duration = end - start;
    std::cout << "Time taken: " << duration.count() << " seconds." << std::endl;

    return 0;
}