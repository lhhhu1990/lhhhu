/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "GapValueAux.h"

#include "MooseMesh.h"
#include "SystemBase.h"
#include "MooseEnum.h"
#include "PenetrationLocator.h"

#include "libmesh/string_to_enum.h"

template<>
InputParameters validParams<GapValueAux>()
{
  MooseEnum orders("FIRST SECOND THIRD FOURTH", "FIRST");

  InputParameters params = validParams<AuxKernel>();
  params.set<bool>("_dual_restrictable") = true;
  params.addRequiredParam<BoundaryName>("paired_boundary", "The boundary on the other side of a gap.");
  params.addRequiredParam<VariableName>("paired_variable", "The variable to get the value of.");
  params.set<bool>("use_displaced_mesh") = true;
  params.addParam<Real>("tangential_tolerance", "Tangential distance to extend edges of contact surfaces");
  params.addParam<Real>("normal_smoothing_distance", "Distance from edge in parametric coordinates over which to smooth contact normal");
  params.addParam<std::string>("normal_smoothing_method","Method to use to smooth normals (edge_based|nodal_normal_based)");
  params.addParam<MooseEnum>("order", orders, "The finite element order");
  params.addParam<bool>("warnings", false, "Whether to output warning messages concerning nodes not being found");
  return params;
}

GapValueAux::GapValueAux(const InputParameters & parameters) :
    AuxKernel(parameters),
    _penetration_locator(_nodal ?  getPenetrationLocator(parameters.get<BoundaryName>("paired_boundary"), boundaryNames()[0], Utility::string_to_enum<Order>(parameters.get<MooseEnum>("order"))) : getQuadraturePenetrationLocator(parameters.get<BoundaryName>("paired_boundary"), boundaryNames()[0], Utility::string_to_enum<Order>(parameters.get<MooseEnum>("order")))),
    _moose_var(_subproblem.getVariable(_tid, getParam<VariableName>("paired_variable"))),
    _serialized_solution(_moose_var.sys().currentSolution()),
    _dof_map(_moose_var.dofMap()),
    _warnings(getParam<bool>("warnings"))
{
  if (parameters.isParamValid("tangential_tolerance"))
    _penetration_locator.setTangentialTolerance(getParam<Real>("tangential_tolerance"));

  if (parameters.isParamValid("normal_smoothing_distance"))
    _penetration_locator.setNormalSmoothingDistance(getParam<Real>("normal_smoothing_distance"));

  if (parameters.isParamValid("normal_smoothing_method"))
    _penetration_locator.setNormalSmoothingMethod(parameters.get<std::string>("normal_smoothing_method"));

  Order pairedVarOrder(_moose_var.order());
  Order gvaOrder(Utility::string_to_enum<Order>(parameters.get<MooseEnum>("order")));
  if (pairedVarOrder != gvaOrder && pairedVarOrder != CONSTANT)
    mooseError2("ERROR: specified order for GapValueAux (", Utility::enum_to_string<Order>(gvaOrder),
                ") does not match order for paired_variable \"", _moose_var.name(), "\" (",
                Utility::enum_to_string<Order>(pairedVarOrder), ")");

}

Real
GapValueAux::computeValue()
{
  const Node * current_node = NULL;

  if (_nodal)
    current_node = _current_node;
  else
    current_node = _mesh.getQuadratureNode(_current_elem, _current_side, _qp);

  PenetrationInfo * pinfo = _penetration_locator._penetration_info[current_node->id()];

  Real gap_value = 0.0;

  if (pinfo)
  {
    std::vector<std::vector<Real> > & side_phi = pinfo->_side_phi;
    if (_moose_var.feType().order != CONSTANT)
      gap_value = _moose_var.getValue(pinfo->_side, side_phi);
    else
      gap_value = _moose_var.getValue(pinfo->_elem, side_phi);
  }
  else
  {
    if (_warnings)
    {
      std::stringstream msg;
      msg << "No gap value information found for node ";
      msg << current_node->id();
      msg << " on processor ";
      msg << processor_id();
      mooseWarning2( msg.str() );
    }
  }
  return gap_value;
}
