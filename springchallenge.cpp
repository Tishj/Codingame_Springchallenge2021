#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <list>

using namespace std;

int factorial(int n)
{
  return (n == 1 || n == 0) ? 1 : factorial(n - 1) * n;
}

std::vector<std::string> split(const std::string& s, char seperator)
{
   std::vector<std::string> output;

    std::string::size_type prev_pos = 0, pos = 0;

    while((pos = s.find(seperator, pos)) != std::string::npos)
    {
        std::string substring( s.substr(prev_pos, pos-prev_pos) );

        output.push_back(substring);

        prev_pos = ++pos;
    }

    output.push_back(s.substr(prev_pos, pos-prev_pos)); // Last word

    return output;
}

enum	e_player
{
	PLAYER_ENEMY,
	PLAYER_FRIENDLY,
	PLAYER_SIZE,
};

enum	e_richness
{
	RICHNESS_UNUSABLE,
	RICHNESS_LOW,
	RICHNESS_MEDIUM,
	RICHNESS_HIGH
};

enum	e_treesize
{
	TREE_DEAD = -1,
	TREE_SEED,
	TREE_SMALL,
	TREE_MEDIUM,
	TREE_LARGE
};

enum	e_sundirection
{
	SUN_RIGHT,
	SUN_UPRIGHT,
	SUN_UPLEFT,
	SUN_LEFT,
	SUN_DOWNLEFT,
	SUN_DOWNRIGHT,
	SUN_SIZE
};

enum	e_action
{
	ACTION_SEED,
	ACTION_GROW,
	ACTION_COMPLETE,
	ACTION_WAIT,
	ACTION_SIZE
};

class	Tree
{
	private:
		e_player	_owner;
		e_treesize	_size;
		size_t		_index;
		bool		_dormant;
		bool		_in_shadow;
	public:
		Tree() : _owner(), _size(), _in_shadow(false)
		{
			size_t	tmp;
			cin >> _index;
			cin >> tmp;
			_size = (e_treesize)tmp;
			cin >> tmp;
			_owner = (e_player)tmp;
			cin >> _dormant; cin.ignore();
		}
		Tree(e_player owner, e_treesize size, size_t index, bool dormant) : _owner(owner),
																			_size(size),
																			_index(index),
																			_dormant(dormant),
																			_in_shadow(false)
		{}
		Tree(const Tree& other) :	_owner(other._owner),
									_size(other._size),
									_index(other._index),
									_dormant(other._dormant),
									_in_shadow(other._in_shadow)
		{}
		size_t	GetIndex()
		{
			return (_index);
		}
		e_treesize	GetSize()
		{
			return (_size);
		}
		void		SetSize(e_treesize size)
		{
			this->_size = size;
		}
		void		SetDormant(bool value)
		{
			_dormant = value;
		}
		bool		GetDormant()
		{
			return (_dormant);
		}
		void		SetShadow(bool value)
		{
			this->_in_shadow = value;
		}
		bool		GetShadow(void)
		{
			return (this->_in_shadow);
		}
		e_player	GetOwner(void)
		{
			return (this->_owner);
		}
};

class	Cell
{
	private:
		size_t			_index;
		e_richness		_richness;
		vector<size_t>	_neighbours;
		Tree			*_tree;
	public:
		Cell() :	_neighbours(SUN_SIZE),
					_tree(NULL)
		{
			size_t	tmp;
			cin >> _index;
			cin >> tmp;
			_richness = (e_richness)tmp;
			for (size_t i = 0; i < SUN_SIZE; i++)
				cin >> _neighbours[i];
			cin.ignore();
		}
		Cell(const Cell& other) :	_index(other._index),
									_richness(other._richness),
									_neighbours(other._neighbours),
									_tree(other._tree)
		{}
		~Cell() {}
		size_t	GetNeighbourIndex(e_sundirection direction)
		{
			return (_neighbours[direction]);
		}
		void	SetTree(Tree *tree)
		{
			this->_tree = tree;
		}
		Tree	*GetTree(void)
		{
			return (this->_tree);
		}
		e_richness	GetRichness(void)
		{
			return (this->_richness);
		}
};

class Game; //forward declare

//needs to know that Game exists, but no specifics.
class	Action
{
	private:
		e_action	_type;
	public:
		virtual bool	apply(e_player player, Game& game) const = 0;
		virtual operator std::string() const = 0;
		virtual ~Action() {}
};

//Game needs to know what action is, because it might need to use functions from Action.
class	Game
{
	private:
		e_sundirection	_sundirection;
		vector<size_t>	_score;
		vector<size_t>	_sun;
		vector<Cell>	_cells;
		size_t			_nutrients;
		bool			_opponent_is_waiting;
		vector<Tree>	_trees;
		vector<Action>	_actions;

		void			AssignTrees()
		{
			for (size_t i = 0; i < _trees.size(); i++)
				_cells[_trees[i].GetIndex()].SetTree(&_trees[i]);
		}
		//effectively remove the tree from the cell
		void			PruneDeadTrees()
		{
			for (size_t i = 0; i < _trees.size(); i++)
			{
				if (_trees[i].GetSize() == TREE_DEAD)
					_cells[_trees[i].GetIndex()].SetTree(NULL);
			}
		}
		void			UpdateSpookyShadows()
		{
			for (size_t i = 0; i < _trees.size(); i++)
			{
				Cell*	shadow_caster = &_cells[_trees[i].GetIndex()];
				for (size_t j = 0; j < _trees[i].GetSize(); j++)
				{
					size_t	neighbour_index = shadow_caster->GetNeighbourIndex(_sundirection);
					Tree*	tree = _cells[neighbour_index].GetTree();
					if (tree && tree->GetSize() <= _trees[i].GetSize())
						tree->SetShadow(true);
					shadow_caster = &_cells[neighbour_index];
				}
			}
		}
	public:
		Game() :	_sundirection(),
					_score(PLAYER_SIZE),
					_sun(PLAYER_SIZE)
		{
			size_t	number_of_cells;
			cin >> number_of_cells; cin.ignore();
			for (size_t i = 0; i < number_of_cells; i++)
				_cells.push_back(Cell());
		}
		Game(const Game& other) :	_sundirection(other._sundirection),
									_score(other._score),
									_sun(other._sun),
									_cells(other._cells),
									_opponent_is_waiting(other._opponent_is_waiting),
									_trees(other._trees)
		{}
		~Game() {}
		void	update()
		{
			size_t	day;
			cin >> day; cin.ignore();
			_sundirection = static_cast<e_sundirection>(day % SUN_SIZE);
			cin >> _nutrients; cin.ignore();
			cin >> _sun[PLAYER_FRIENDLY]; cin >> _score[PLAYER_FRIENDLY]; cin.ignore();
			cin >> _sun[PLAYER_ENEMY]; cin >> _score[PLAYER_ENEMY]; cin >> _opponent_is_waiting; cin.ignore();

			size_t	number_of_trees;
			cin >> number_of_trees;
			_trees.clear();
			for (size_t i = 0; i < number_of_trees; i++)
				_trees.push_back(Tree());
			AssignTrees();
		}
		size_t	CountTreesOfSize(e_treesize size)
		{
			size_t	count = 0;
			for (size_t i = 0; i < _trees.size(); i++)
				count += (_trees[i].GetSize() == size);
			return (count);
		}
		size_t	GetScore(e_player player)
		{
			return (_score[player]);
		}
		size_t	GetSun(e_player player)
		{
			return (_sun[player]);
		}
		e_richness	GetCellRichness(size_t index)
		{
			return (_cells[index].GetRichness());
		}
		Cell&		GetCell(size_t index)
		{
			return (_cells[index]);
		}
		bool		PlantSeed(e_player player, Tree new_tree, Cell& cell)
		{
			if (cell.GetRichness() == RICHNESS_UNUSABLE)
				return (false);
			size_t	cost;
			cost = CountTreesOfSize(TREE_SEED);
			if (_sun[player] < cost)
				return (false);
			cell.GetTree()->SetDormant(true);
			return (true);
		}
		bool		HarvestTree(e_player player, size_t tree_index)
		{
			Tree*	tree;

			tree = _cells[tree_index].GetTree();
			if (!tree || tree->GetSize() != TREE_LARGE || _cells[tree_index].GetRichness() == RICHNESS_UNUSABLE)
				return (false);
			if (_sun[player] < 4)
				return (false);
			
			_score[player] += ((_cells[tree_index].GetRichness() - 1) * 2) + _nutrients;
			if (_nutrients)
				_nutrients--;
			tree->SetSize(TREE_DEAD);
			return (true);
		}
		size_t	CalculateSunGain(e_player player)
		{
			size_t	sun_gained = 0;

			for (size_t i = 0; i < _trees.size(); i++)
				sun_gained += (_trees[i].GetOwner() == player && _trees[i].GetShadow() == false) * _trees[i].GetSize();
			return (sun_gained);
		}
};

//-----------------------DERIVED ACTION CLASSES-----------------------

class	Seed : public Action
{
	private:
		Seed() {}
		size_t	_tree_index;
		size_t	_seed_index;
	public:
		Seed(size_t tree_index, size_t seed_index) :	_tree_index(tree_index),
														_seed_index(seed_index)
		{}
		Seed(const Seed& other) :	_tree_index(other._tree_index),
									_seed_index(other._seed_index)
		{}
		virtual ~Seed() {}
		bool	apply(e_player player, Game& game) const
		{
			return (game.PlantSeed(player, Tree(player, TREE_SEED, _seed_index, true), game.GetCell(_tree_index)));
		}
		operator std::string() const
		{
			return ("SEED " + to_string(_tree_index) + " " + to_string(_seed_index));
		}
};

class	Grow : public Action
{
	private:
		Grow() {}
		size_t	_index;
	public:
		Grow(size_t index) : _index(index) {}
		virtual ~Grow() {}
		Grow(const Grow& other) : _index(other._index) {}
		bool	apply(e_player player, Game& game) const
		{
			Tree*	tree;
			size_t	current_size;

			tree = game.GetCell(_index).GetTree();
			current_size = tree->GetSize();
			if (!tree || current_size == TREE_LARGE)
				return (false);
			size_t	cost = game.CountTreesOfSize(static_cast<e_treesize>(current_size + 1));
			cost += (factorial(current_size + 1) + !!current_size);
			if (game.GetSun(player) < cost)
				return (false);
			tree->SetSize(static_cast<e_treesize>(current_size + 1));
			return (true);
		}
		operator std::string() const
		{
			return ("GROW " + to_string(_index));
		}
};

class	Complete : public Action
{
	private:
		Complete() {}
		size_t	_index;
	public:
		Complete(size_t index) : _index(index) {}
		Complete(const Complete& other) :	_index(other._index) {}
		virtual ~Complete() {}
		bool	apply(e_player player, Game& game) const
		{
			return (game.HarvestTree(player, _index));
		}
		operator std::string() const
		{
			return ("COMPLETE " + to_string(_index));
		}
};

class	Wait : public Action
{
	public:
		Wait() {}
		virtual ~Wait() {}
		Wait(const Wait& other) {}
		bool	apply(e_player player, Game& game) const
		{
			return (true);
		}
		operator std::string() const
		{
			return ("WAIT");
		}
};

class	ActionBuilder
{
	public:
		ActionBuilder() {}
		Action*	build(std::string format)
		{
			if (format == "WAIT")
				return (new Wait());
			if (format.size() >= 4 && format.substr(0,4) == "SEED")
			{
				vector<string>	parts = split(format, ' ');
				if (parts.size() == 3)
					return (new Seed(stoi(parts[1]), stoi(parts[2])));
			}
			if (format.size() >= 4 && format.substr(0,4) == "GROW")
			{
				vector<string>	parts = split(format, ' ');
				if (parts.size() == 2)
					return (new Grow(stoi(parts[1])));
			}
			if (format.size() >= sizeof("COMPLETE") && format.substr(0, sizeof("COMPLETE")) == "COMPLETE")
			{
				vector<string>	parts = split(format, ' ');
				if (parts.size() == 2)
					return (new Complete(stoi(parts[1])));
			}
			cerr << "FORMAT NOT RECOGNIZED!" << std::endl;
			return (NULL);
		}
};

int main()
{
	Game			game;
	vector<Action*>	actions;
	ActionBuilder	action_builder;

    while (1)
	{
		game.update();
		for (size_t i = 0; i < actions.size(); i++)
			delete actions[i];
		actions.clear();
        int number_of_possible_actions; // all legal actions
        cin >> number_of_possible_actions; cin.ignore();
        for (int i = 0; i < number_of_possible_actions; i++)
		{
            string possible_action;
            getline(cin, possible_action); // try printing something from here to start with
			actions.push_back(action_builder.build(possible_action));
			cerr << (std::string)*(actions.back()) << std::endl;
			if (i + 1 == number_of_possible_actions)
				cout << (std::string)*(actions.back()) << std::endl;
        }
    }
}
