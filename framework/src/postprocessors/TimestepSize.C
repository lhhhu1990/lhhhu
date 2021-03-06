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

#include "TimestepSize.h"
#include "FEProblem.h"

template<>
InputParameters validParams<TimestepSize>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  return params;
}

TimestepSize::TimestepSize(const InputParameters & parameters) :
    GeneralPostprocessor(parameters),
    _feproblem(dynamic_cast<FEProblemBase &>(_subproblem))
{}

Real
TimestepSize::getValue()
{
  return _feproblem.dt();
}

