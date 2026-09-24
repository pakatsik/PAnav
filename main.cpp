#include <iostream>
#include <algorithm>
#include <iomanip>

#include "SNNM.h"
#include "ShipModel.h"
#include "Routing.h"
#include "Performance.h"
#include "Wind.h"

//Helper Functions:
//Find group's row index
int getIndexOfGroup(int _matrix[16][5], int _target[5])
{
	//Sort first into matching format (Descending order)
	std::sort(_target, _target + 5, std::greater<int>());

	bool matched;
	for (int i = 0; i < 16; i++)
	{
		matched = true;

		for (int j = 0; j < 5; j++)
		{

			if (!(_matrix[i][j] == _target[j]))
			{
				matched = false;
			}

		}

		if (matched == true)
		{
			return i;
		}
	}

	return -1;
}
//print for either int or double or array3d
void printMatrix16x5(double _matrix[16][5])
{
	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			std::cout << _matrix[i][j] << " | ";
		}
		std::cout << "\n";
	}
}
void printMatrix16x5(int _matrix[16][5])
{
	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			std::cout << _matrix[i][j] << " | ";
		}
		std::cout << "\n";
	}
}
void printMatrix16x5(int _matrix[16][5][2])
{
	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			std::cout << "(" << _matrix[i][j][0] << ",";
			std::cout << _matrix[i][j][1] << ")| ";
		}
		std::cout << "\n";
	}
}
//same but for 5x5
void printMatrix5x5(double _matrix[5][5])
{
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			std::cout << _matrix[i][j] << " | ";
		}
		std::cout << "\n";
	}
}
void printMatrix5x5(int _matrix[5][5])
{
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			std::cout << _matrix[i][j] << " | ";
		}
		std::cout << "\n";
	}
}


//Optimizer Functions:
//Optimization Algorithm (inputs travelCosts, groupsIndex, empty Bellman, empty Decisions | output void, but fills empty matrixes)
//Note: Contains tons of debug text left in to show algorithm process. All useless debugs are marked with "//Debug"
void solveForOptimal(double _travelCosts[5][5], int _index[16][5], double(&_minCosts)[16][5], int(&_decisions)[16][5][2])
{
	//Temp array for finding next group
	int tempArray[5] = {};
	//Temp cost total
	double tempCost = 0;
	//Temp target index
	int nextGroupIndex = 0;
	//Calculation for every island in every group, using the index
	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			//Edge Cases:
			//Island is 1
			if (_index[i][j] == 1)
			{
				if (i == 0 && j == 0)
				{
					//Last travel cost to end, has essentially arrived, is zero
					_minCosts[i][j] = 0;
					continue;
				}
				if (i == 15 && j == 4)
				{
					std::cout << "Row: " << i << " Col: " << j << "\n";//Debug

					//First travel from 1 to any island should not remove 1 from group - Same row calculation
					//This essentially calculates the FINAL minCOST and first move.
					nextGroupIndex = i;
					//Save minimum + corresponding best decision, after calculating for all targets
					for (int z = 0; z < 5; z++)
					{
						//Don't do for target zero, or target 1)
						if (_index[nextGroupIndex][z] == 0 || _index[nextGroupIndex][z] == 1) { break; }

						//Add travel cost to the targets calculated minimum
						tempCost = _travelCosts[_index[i][j] - 1][_index[nextGroupIndex][z] - 1] + _minCosts[nextGroupIndex][z];

						std::cout << "Calculated cost to go from: " << _index[i][j] << " to " << _index[nextGroupIndex][z]//Debug
							<< " = " << _travelCosts[_index[i][j] - 1][_index[nextGroupIndex][z] - 1]//Debug
								<< " + minimum from there = " << _minCosts[nextGroupIndex][z]//Debug
								<< " for a total of " << tempCost << "\n\n";//Debug

						//Save this value? Only if first calculated or lowest
						if (z == 0 || tempCost < _minCosts[i][j])
						{
							_minCosts[i][j] = tempCost;
							_decisions[i][j][0] = nextGroupIndex;
							_decisions[i][j][1] = z;
						}
					}

					std::cout << "First Decision = " << _index[_decisions[i][j][0]][_decisions[i][j][1]] << "\n";//Debug
					std::cout << "Minimum Cost = " << _minCosts[i][j] << "\n----------------------------------"//Debug
						<< "----------------------------------\n";

					continue;
				}
				//Only is at island N.1 at the start and at the end, so will never land here otherwise, no need to calculate, skip
				continue;
			}
			//Island is 0
			if (_index[i][j] == 0)
			{
				//If zero, invalid island (empty slot in group), skip
				continue;
			}

			std::cout << "Row: " << i << " Col: " << j << "\n";//Debug

			//Find next group (Current group after removing current island)

			std::cout << "Current group is: {";//Debug

			for (int z = 0; z < 5; z++)
			{
				tempArray[z] = _index[i][z];
				std::cout << tempArray[z] << ",";
			}

			std::cout << "}";//Debug

			//Then find and remove current island from array

			std::cout << " Target group is: {";//Debug

			for (int z = 0; z < 5; z++)
			{
				//If groupIndexArray element is equal to current element, remove it
				if (tempArray[z] == _index[i][j])
				{
					tempArray[z] = 0;
				}
				std::cout << tempArray[z] << ",";//Debug
			}

			std::cout << "}\n";//Debug

			std::cout << "Corresponding to group of index: " << getIndexOfGroup(_index, tempArray) << "\n\n";//Debug

			nextGroupIndex = getIndexOfGroup(_index, tempArray);
			//Save minimum + corresponding best decision, after calculating for all targets
			for (int z = 0; z < 5; z++)
			{
				//Don't do for target zero, or target 1 unless nextGroup is index 0 (last travel)
				if (_index[nextGroupIndex][z] == 0 || (_index[nextGroupIndex][z] == 1 && !(nextGroupIndex == 0))) { break; }

				//Add travel cost to the targets calculated minimum
				tempCost = _travelCosts[_index[i][j] - 1][_index[nextGroupIndex][z] - 1] + _minCosts[nextGroupIndex][z];

				std::cout << "Calculated cost to go from: " << _index[i][j] << " to " << _index[nextGroupIndex][z]//Debug
					<< " = " << _travelCosts[_index[i][j] - 1][_index[nextGroupIndex][z] - 1]//Debug
						<< " + minimum from there = " << _minCosts[nextGroupIndex][z]//Debug
						<< " for a total of " << tempCost << "\n\n";//Debug

				//Save this value? Only if first calculated or lowest
				if (z == 0 || tempCost < _minCosts[i][j])
				{
					_minCosts[i][j] = tempCost;
					_decisions[i][j][0] = nextGroupIndex;
					_decisions[i][j][1] = z;
				}
			}

			std::cout << "Best Decision = " << _index[_decisions[i][j][0]][_decisions[i][j][1]] << "\n";//Debug
			std::cout << "Minimum Cost = " << _minCosts[i][j] << "\n----------------------------------"//Debug
					  << "----------------------------------\n";
		}

	}
}
//Print Optimal Route
void printOptimalRoute(int _decisions[16][5][2], int _index[16][5])
{
	int lastPosition[2] = { 15,4 };
	int nextPosition[2] = {};
	std::cout << "1";
	for (int i = 1; i < 6; i++)
	{
		nextPosition[0] = _decisions[lastPosition[0]][lastPosition[1]][0];
		nextPosition[1] = _decisions[lastPosition[0]][lastPosition[1]][1];
		std::cout << "-->" << _index[nextPosition[0]][nextPosition[1]];
		lastPosition[0] = nextPosition[0];
		lastPosition[1] = nextPosition[1];
	}
	std::cout << "\n";
}


int main() 
{
	//Part 1, Travel Cost calculation given Islands and Weather
	//Required Output from part 1
	double travelCosts[5][5];
	{
		using namespace std;
		ShipModel ship;

		ship.Lpp = 225.10;
		ship.beam = 32.26;
		ship.draft = 12.20;       // fixed loading condition
		ship.Cb = 0.8656;
		ship.serviceSpeed = 14.5;

		// Approximations used because detailed hull geometry is unavailable
		ship.Le = 45.0;
		ship.Lr = 56.3;
		ship.kyyRatio = 0.25;
		ship.frontalWindArea = 500.0;

		WeatherData weather;

		cout << "SHIP WEATHER TRAVEL COST CALCULATION\n";
		cout << "===============================\n\n";

		cout << "Enter significant wave height Hs [m]: [Example: 3] ";
		cin >> weather.Hs;

		cout << "Enter peak wave period Tp [s]: [Example: 8]";
		cin >> weather.Tp;

		cout << "Enter wave direction [deg]: [Example: 45]";
		cin >> weather.waveDirection;

		cout << "Enter wind speed [m/s]: [Example: 7]";
		cin >> weather.windSpeed;

		cout << "Enter wind direction [deg]: [Example: 30]";
		cin >> weather.windDirection;

		if (weather.Hs < 0.0)
		{
			cout << "Error: Hs cannot be negative.\n";
			return 1;
		}

		if (weather.Tp <= 0.0)
		{
			cout << "Error: Tp must be greater than zero.\n";
			return 1;
		}

		if (weather.windSpeed < 0.0)
		{
			cout << "Error: Wind speed cannot be negative.\n";
			return 1;
		}

		const int NUMBER_OF_ISLANDS = 5;
		Waypoint islands[NUMBER_OF_ISLANDS];

		islands[0].name = "Kea";
		islands[0].latitude = 37.62;
		islands[0].longitude = 24.32;

		islands[1].name = "Syros";
		islands[1].latitude = 37.44;
		islands[1].longitude = 24.94;

		islands[2].name = "Paros";
		islands[2].latitude = 37.09;
		islands[2].longitude = 25.15;

		islands[3].name = "Naxos";
		islands[3].latitude = 37.10;
		islands[3].longitude = 25.38;

		islands[4].name = "Santorini";
		islands[4].latitude = 36.42;
		islands[4].longitude = 25.43;

		for (int i = 0; i < NUMBER_OF_ISLANDS; i++)
		{
			for (int j = 0; j < NUMBER_OF_ISLANDS; j++)
			{
				if (i == j)
				{
					travelCosts[i][j] = 0.0;
				}
				else
				{
					RouteLeg leg = calculateRouteLeg(
						islands[i], islands[j], ship, weather);

					travelCosts[i][j] = leg.travelTimeHours;
				}
			}
		}

		cout << "\nTRAVEL TIME MATRIX [hours]\n";
		cout << "==========================\n\n";

		cout << setw(12) << "FROM / TO";
		for (int j = 0; j < NUMBER_OF_ISLANDS; j++)
			cout << setw(12) << islands[j].name;

		cout << "\n";

		for (int i = 0; i < NUMBER_OF_ISLANDS; i++)
		{
			cout << setw(12) << islands[i].name;

			for (int j = 0; j < NUMBER_OF_ISLANDS; j++)
			{
				if (i == j)
				{
					cout << setw(12) << "-";
				}
				else
				{
					cout << setw(12) << fixed << setprecision(2)
						<< travelCosts[i][j];
				}
			}

			cout << "\n";
		}

	}

	//Part 2, Optimization
	std::cout << "\n\nSTART OF OPTIMIZATION PROCESS\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\n\n";

	//Travel Costs calculated by Part 1 (double travelCosts[5][5])

	//List all possible groups of islands, each with unique index its row number
	int groupsIndex[16][5] = {
		{1,0,0,0,0},
		{2,1,0,0,0},
		{3,1,0,0,0},
		{4,1,0,0,0},
		{5,1,0,0,0},
		{3,2,1,0,0},
		{4,2,1,0,0},
		{5,2,1,0,0},
		{4,3,1,0,0},
		{5,3,1,0,0},
		{5,4,1,0,0},
		{4,3,2,1,0},
		{5,3,2,1,0},
		{5,4,2,1,0},
		{5,4,3,1,0},
		{5,4,3,2,1},
	};

	//Create Empty BellmanMatrix
	double minCostMatrix[16][5] = {};
	//Create Empty DecisionsMatrix (Which island to go from here? In the form of row,col position)
	int decisionsMatrix[16][5][2] = {};
	
	//Solve (Inputs are travelCosts and the index, outputs are the matrixes (passed as references and filled))
	solveForOptimal(travelCosts, groupsIndex, minCostMatrix, decisionsMatrix);

	std::cout << "END OF OPTIMIZATION PROCESS\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\n\n";

	std::cout << "PRODUCED RESULTS:\n\n";

	std::cout << "Cost Matrix:\n";
	printMatrix16x5(minCostMatrix);

	std::cout << "\nDecision Matrix:\n";
	printMatrix16x5(decisionsMatrix);

	std::cout << "\nOptimal Route: ";
	printOptimalRoute(decisionsMatrix, groupsIndex);

	std::cout << "\nMinimum Cost: " << minCostMatrix[15][4] << " hours\n";

	std::cout << "\n\n\n"
		<< "       _                        \n"
		<< "       \\`*-.                    \n"
		<< "        )  _`-.                 \n"
		<< "       .  : `. .                \n"
		<< "       : _   '  \\               \n"
		<< "       ; *` _.   `*-._          \n"
		<< "       `-.-'          `-.       \n"
		<< "         ;       `       `.     \n"
		<< "         :.       .        \\    \n"
		<< "         . \\  .   :   .-'   .   \n"
		<< "         '  `+.;  ;  '      :   \n"
		<< "         :  '  |    ;       ;-. \n"
		<< "         ; '   : :`-:     _.`* ;\n"
		<< "[bug] .*' /  .*' ; .*`- +'  `*' \n"
		<< "      `*-*   `*-*  `*-*'        \n";//https://www.asciiart.eu/art/40e94553cf590a1d

	return 0;
}