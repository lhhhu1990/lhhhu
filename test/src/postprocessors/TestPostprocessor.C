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

#include "TestPostprocessor.h"

template<>
InputParameters validParams<TestPostprocessor>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  MooseEnum test_type("grow use_older_value report_old");
  params.addRequiredParam<MooseEnum>("test_type", test_type, "The type of test to perform");
  params.addParam<PostprocessorName>("report_name", "The name of the postprocessor value to report");
  return params;
}

TestPostprocessor::TestPostprocessor(const InputParameters & parameters) :
    GeneralPostprocessor(parameters),
    _test_type(getParam<MooseEnum>("test_type")),
    _old_val(getPostprocessorValueOldByName(name())),
    _older_val(getPostprocessorValueOlderByName(name()))
{
  if (_test_type == "report_old" && !isParamValid("report_name"))
    mooseError2("Must set 'report_name' parameter when using the 'report_old' test type.");
}

Real
TestPostprocessor::getValue()
{
  if (_test_type == "grow")
  {
    if (_t_step == 0)
      return 1;
    return _old_val + 1;
  }

  else if (_test_type == "use_older_value")
  {
    if (_t_step == 0)
      return 1;
    return _old_val + _older_val;
  }

  else if (_test_type == "report_old")
    return getPostprocessorValueOld("report_name");

  // This should not be attainable
  else
  {
    mooseError2("Invalid test type.");
    return 0.0;
  }
}
