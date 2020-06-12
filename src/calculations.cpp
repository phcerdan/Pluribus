#include "holdem.h"
#include "calculations.h"


unsigned long long compress_hand_lossless(unsigned long long hand) {
    /*
        A lossless compression of a poker hand exploits the fact that suits are
        indifferent; the only thing that is important is to keep the same cards in the
        same groups.
    */
    unsigned long long compressed_hand = 0ULL;
    unsigned long long value_mask = 0x0001111111111111;
    std::vector< std::pair<unsigned long long, int> > suit_value = {
                                                        std::make_pair(0ULL, 0),
                                                        std::make_pair(0ULL, 1),
                                                        std::make_pair(0ULL, 2),
                                                        std::make_pair(0ULL, 3)
                                                        };

    for (int suit = 0; suit < 4; ++suit)
        suit_value[suit].first = (hand>>suit) & value_mask;

    sort(
        suit_value.begin(),
        suit_value.end(),
        [](const std::pair<unsigned long long, int> &l, std::pair<unsigned long long, int> &r) -> bool {
            return l.first > r.first;
        }
    );

    for (int suit = 0; suit < 4; ++suit) {
        if (suit - suit_value[suit].second >= 0)
            compressed_hand |= (hand<<(suit - suit_value[suit].second)) & (value_mask<<suit);
        else
            compressed_hand |= (hand>>(suit_value[suit].second - suit)) & (value_mask<<suit);
    }

    return compressed_hand;
}

namespace calculations {

    std::unordered_map< unsigned long long, int > suit_permutations;

    void calculate_suit_permutations(unsigned long long current_hand, int upper_bound, int max_cards) {
        if (max_cards == 0) {
            unsigned long long compressed_hand = compress_hand_lossless(current_hand);
            if (calculations::suit_permutations.find(compressed_hand) != calculations::suit_permutations.end())
                calculations::suit_permutations[compressed_hand]++;
            else
                calculations::suit_permutations[compressed_hand] = 1;
        } else {
            for (int x = 0; x < upper_bound; x++) {
                if (current_hand & 1ULL<<x)
                    continue;
                calculations::calculate_suit_permutations(current_hand | 1ULL<<x, x, max_cards - 1);
            }
        }
    }

    void load_suit_permutations() {
        std::string file_name = "../files/suit_permutations.txt";

        if (!calculations::suit_permutations.empty())
            return;

        if (std::experimental::filesystem::exists(file_name)) {
            std::ifstream ifs(file_name);
            boost::archive::text_iarchive ia(ifs);
            ia >> calculations::suit_permutations;
            ifs.close();
        } else {
            for (int max_cards = 1; max_cards <= 7; ++max_cards)
                calculations::calculate_suit_permutations(0ULL, 52, max_cards);
            std::ofstream ofs(file_name);
            boost::archive::text_oarchive oa(ofs);
            oa << calculations::suit_permutations;
            ofs.close();
        }
    }

    int num_suit_permutations(unsigned long long hand) {
        if (calculations::suit_permutations.empty())
            calculations::load_suit_permutations();

        return calculations::suit_permutations[compress_hand_lossless(hand)];
    }

} // namespace calculations

