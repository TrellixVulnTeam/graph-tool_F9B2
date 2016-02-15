// graph-tool -- a general graph modification and manipulation thingy
//
// Copyright (C) 2006-2016 Tiago de Paula Peixoto <tiago@skewed.de>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

#ifndef GRAPH_BLOCKMODEL_MULTICANONICAL_HH
#define GRAPH_BLOCKMODEL_MULTICANONICAL_HH

#include "config.h"

#include <vector>

#include "graph_tool.hh"
#include "graph_state.hh"
#include "graph_blockmodel_util.hh"
#include <boost/mpl/vector.hpp>

namespace graph_tool
{
using namespace boost;
using namespace std;

#define MULTICANONICAL_BLOCK_STATE_params(State)                               \
    ((__class__,&, mpl::vector<python::object>, 1))                            \
    ((state, &, State&, 0))                                                    \
    ((hist, &, std::vector<size_t>&, 0))                                       \
    ((dens, &, std::vector<double>&, 0))                                       \
    ((S_min, , double, 0))                                                     \
    ((S_max, , double, 0))                                                     \
    ((f, , double, 0))                                                         \
    ((S, , double, 0))                                                         \
    ((E,, size_t, 0))                                                          \
    ((vlist,&, std::vector<size_t>&, 0))                                       \
    ((block_list,&, std::vector<size_t>&, 0))                                  \
    ((c,, double, 0))                                                          \
    ((multigraph,, bool, 0))                                                   \
    ((dense,, bool, 0))                                                        \
    ((partition_dl,, bool, 0))                                                 \
    ((degree_dl,, bool, 0))                                                    \
    ((edges_dl,, bool, 0))                                                     \
    ((allow_empty,, bool, 0))                                                  \
    ((verbose,, bool, 0))                                                      \
    ((niter,, size_t, 0))


template <class State>
struct Multicanonical
{
    GEN_STATE_BASE(MulticanonicalBlockStateBase,
                   MULTICANONICAL_BLOCK_STATE_params(State))

    template <class... Ts>
    class MulticanonicalBlockState
        : public MulticanonicalBlockStateBase<Ts...>
    {
    public:
        GET_PARAMS_USING(MulticanonicalBlockStateBase<Ts...>,
                         MULTICANONICAL_BLOCK_STATE_params(State))
        GET_PARAMS_TYPEDEF(Ts, MULTICANONICAL_BLOCK_STATE_params(State))

        template <class... ATs,
                  typename std::enable_if_t<sizeof...(ATs) ==
                                            sizeof...(Ts)>* = nullptr>
        MulticanonicalBlockState(ATs&&... as)
           : MulticanonicalBlockStateBase<Ts...>(as...),
            _g(_state._g)
        {
            _state.init_mcmc(_c, _partition_dl || _degree_dl || _edges_dl);
        }

        typename state_t::g_t& _g;

        size_t node_state(size_t v)
        {
            return _state._b[v];
        }

        template <class RNG>
        size_t move_proposal(size_t v, RNG& rng)
        {
            auto r = _state._b[v];

            size_t s = _state.sample_block(v, _c, _block_list, rng);

            if (_state._bclabel[s] != _state._bclabel[r])
                return r;

            if (!_allow_empty && _state.virtual_remove_size(v) == 0)
                return r;

            return s;
        }

        std::pair<double, double> virtual_move_dS(size_t v, size_t nr)
        {
            double dS = _state.virtual_move(v, nr, _dense, _multigraph,
                                            _partition_dl, _degree_dl,
                                            _edges_dl);

            double a = 0;
            if (!std::isinf(_c))
            {
                size_t r = _state._b[v];
                double pf = _state.get_move_prob(v, r, nr, _c, false);
                double pb = _state.get_move_prob(v, nr, r, _c, true);
                a = log(pb) - log(pf);
            }
            return make_pair(dS, a);
        }

        void perform_move(size_t v, size_t nr)
        {
            _state.move_vertex(v, nr);
        }
    };
};


} // graph_tool namespace

#endif //GRAPH_BLOCKMODEL_MULTICANONICAL_HH
