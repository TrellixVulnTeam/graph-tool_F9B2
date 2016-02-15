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

#ifndef GRAPH_BLOCKMODEL_MCMC_HH
#define GRAPH_BLOCKMODEL_MCMC_HH

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

#define MCMC_BLOCK_STATE_params(State)                                         \
    ((__class__,&, mpl::vector<python::object>, 1))                            \
    ((state, &, State&, 0))                                                    \
    ((E,, size_t, 0))                                                          \
    ((vlist,&, std::vector<size_t>&, 0))                                       \
    ((block_list,&, std::vector<size_t>&, 0))                                  \
    ((beta,, double, 0))                                                       \
    ((c,, double, 0))                                                          \
    ((multigraph,, bool, 0))                                                   \
    ((dense,, bool, 0))                                                        \
    ((partition_dl,, bool, 0))                                                 \
    ((degree_dl,, bool, 0))                                                    \
    ((edges_dl,, bool, 0))                                                     \
    ((allow_empty,, bool, 0))                                                  \
    ((parallel,, bool, 0))                                                     \
    ((sequential,, bool, 0))                                                   \
    ((verbose,, bool, 0))                                                      \
    ((niter,, size_t, 0))


template <class State, template <class Graph> class MEntries = EntrySet>
struct MCMC
{
    GEN_STATE_BASE(MCMCBlockStateBase, MCMC_BLOCK_STATE_params(State))

    template <class... Ts>
    class MCMCBlockState
        : public MCMCBlockStateBase<Ts...>
    {
    public:
        GET_PARAMS_USING(MCMCBlockStateBase<Ts...>,
                         MCMC_BLOCK_STATE_params(State))
        GET_PARAMS_TYPEDEF(Ts, MCMC_BLOCK_STATE_params(State))

        template <class... ATs,
                  typename std::enable_if_t<sizeof...(ATs) ==
                                            sizeof...(Ts)>* = nullptr>
        MCMCBlockState(ATs&&... as)
           : MCMCBlockStateBase<Ts...>(as...),
            _g(_state._g),
            _m_entries(num_vertices(_state._bg))
        {
            _state.init_mcmc(_c, _partition_dl || _degree_dl || _edges_dl);
        }

        typename state_t::g_t& _g;
        MEntries<typename state_t::g_t> _m_entries;

        size_t node_state(size_t v)
        {
            return _state._b[v];
        }

        template <class RNG>
        size_t move_proposal(size_t v, RNG& rng)
        {
            auto r = _state._b[v];

            if (!_allow_empty && _state.is_last(v))
                return r;

            size_t s = _state.sample_block(v, _c, _block_list, rng);

            if (_state._bclabel[s] != _state._bclabel[r])
                return r;

            return s;
        }

        std::pair<double, double> virtual_move_dS(size_t v, size_t nr)
        {
            double dS = _state.virtual_move(v, nr, _dense, _multigraph,
                                            _partition_dl, _degree_dl,
                                            _edges_dl, _m_entries);
            double a = 0;
            if (!std::isinf(_c))
            {
                size_t r = _state._b[v];
                double pf = _state.get_move_prob(v, r, nr, _c, false,
                                                 _m_entries);
                double pb = _state.get_move_prob(v, nr, r, _c, true,
                                                 _m_entries);
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

#endif //GRAPH_BLOCKMODEL_MCMC_HH
