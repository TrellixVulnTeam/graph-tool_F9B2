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

#include "graph_tool.hh"
#include "random.hh"

#include <boost/python.hpp>

#include "graph_blockmodel_util.hh"
#include "graph_blockmodel.hh"
#define BASE_STATE_params BLOCK_STATE_params
#include "graph_blockmodel_layers.hh"
#include "graph_blockmodel_gibbs.hh"
#include "gibbs_loop.hh"

using namespace boost;
using namespace graph_tool;

GEN_DISPATCH(block_state, BlockState, BLOCK_STATE_params)

template <class BaseState>
GEN_DISPATCH(layered_block_state, Layers<BaseState>::template LayeredBlockState,
             LAYERED_BLOCK_STATE_params)

template <class State>
GEN_DISPATCH(gibbs_block_state, Gibbs<State>::template GibbsBlockState,
             GIBBS_BLOCK_STATE_params(State))

python::object gibbs_layered_sweep(python::object ogibbs_state,
                                   python::object olayered_state,
                                   rng_t& rng)
{
    python::object ret;
    auto dispatch = [&](auto* block_state)
    {
        typedef typename std::remove_pointer<decltype(block_state)>::type
            state_t;

        layered_block_state<state_t>::dispatch
            (olayered_state,
             [&](auto& ls)
             {
                 typedef typename std::remove_reference<decltype(ls)>::type
                     layered_state_t;

                 gibbs_block_state<layered_state_t>::make_dispatch
                     (ogibbs_state,
                      [&](auto& s)
                      {
                          auto ret_ = gibbs_sweep(s, rng);
                          ret = python::make_tuple(ret_.first, ret_.second);
                      });
             },
             false);
    };
    block_state::dispatch(dispatch);
    return ret;
}

void export_layered_blockmodel_gibbs()
{
    using namespace boost::python;
    def("gibbs_layered_sweep", &gibbs_layered_sweep);
}
