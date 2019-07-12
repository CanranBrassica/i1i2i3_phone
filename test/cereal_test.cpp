#include <cereal/archives/binary.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <sstream>
#include <vector>

int main()
{
    std::stringstream ss;

    {
        std::string data = "hogehoge";
        std::vector<char> vec = {'a', 'b', 'c'};

        cereal::BinaryOutputArchive ar{ss};
        ar(data);
        ar(vec);
    }

    std::cout << "{";
    for (char c : ss.str()) {
        std::cout << +c << ", ";
    }
    std::cout << "}" << std::endl;

    {
        cereal::BinaryInputArchive ar{ss};
        std::string data;
        std::vector<char> vec;

        ar(data);
        std::cout << data << std::endl;

        std::cout << "{";
        for (char c : ss.str()) {
            std::cout << +c << ", ";
        }
        std::cout << "}" << std::endl;


        ar(vec);

        for(char c : vec){
            std::cout << +c << ", ";
        }
        std::cout << std::endl;

        std::cout << "{";
        for (char c : ss.str()) {
            std::cout << +c << ", ";
        }
        std::cout << "}" << std::endl;
    }

}
