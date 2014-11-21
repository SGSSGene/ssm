#!/usr/bin/perl -w

use strict;

use Data::Dumper;
use Getopt::Long;
use File::Basename;


my @g_exportMachines;
my $g_includeSSM = "\"ssm.h\"";
my $g_inputFile = "";
my $g_dirname = "";

#Regex to identifie words, datatypes etc...
	my $word = '[_a-zA-Z][_a-zA-Z0-9]*';
	my $wordRegEx = $word;
	my $dt_bool    = '(?:true|false)';
	my $dt_int     = '[+-]?[0-9]+';
	my $dt_enum    = "(?:$word\:\:)*$word";
	my $dt_float   = '(?:[+-]?[0-9]+\.[0-9]*|\.[0-9]+)';
	my $dt_char    = '\'[^\']\'';
	my $dt_str     = '\"[^\"]*\"';
	my $dt_any     = "(?:$dt_bool|$dt_int|$dt_float|$dt_str|$dt_enum|$dt_char)";
	my $call       = '\((?:\s*'."(?:$dt_any(?:\\s*\\,\\s*$dt_any)*)?".')\s*\)';
	my $function   = "$word(?:$call)?(?:(?:\.|->)$word(?:$call)?)*";
	my $functionRegEx = $function;
	my $condition = "(?:!?$function|$function\\s*(?:==|!=|>|<|>=|<=)\\s*$dt_any)";
	my $conditionRegEx = $condition;


# Taking a function call appart
sub analyseFunction {
	my $function = shift;
	my $functionSignature = "";
	my $functionValues    = "";
	my $functionName      = "";
	my $functionNameGet   = "";
	my $callNbr = -1;

	while ($function) {
		$function =~ /^($word)(.*)$/;
		$function = $2;
		$functionName .= $1;
		$functionNameGet .= $1;

		if ($function =~ /^($call)(.*)$/) {
			$function = $2;
			$functionNameGet .= '(';
			my $callValue = $1;
			$callValue =~ /^\((.*)\)$/;
			$callValue = $1;
			while ($callValue =~ /^\s*($dt_any)\s*(?:,(.*))?$/) {

				my $parameter = $1;
				$callValue = $2 if $2;
				$callValue = "" unless $2;
				if ($parameter =~ /^$/) {
				} elsif ($parameter =~ /^($dt_bool)$/) {
					$functionSignature .= "bool, ";
					$functionValues    .= "$1, ";
					$functionName .= "_b";

					$callNbr++;
					$functionNameGet .= "get<$callNbr>(p)";

				} elsif ($parameter =~ /^($dt_int)$/) {
					$functionSignature .= "int, ";
					$functionValues    .= "$1, ";
					$functionName .= "_i";

					$callNbr++;
					$functionNameGet .= "get<$callNbr>(p)";
				} elsif ($parameter =~ /^($dt_enum)$/) { # This also covers the nullptr case
					$parameter =~ /^(?:$word\:\:)*($word)$/;
					$functionName .= "_$1";

					$functionNameGet .= "$parameter";

				} elsif ($parameter =~ /^($dt_float)$/) {
					$functionSignature .= "double, ";
					$functionValues    .= "$1, ";
					$functionName .= "_f";

					$callNbr++;
					$functionNameGet .= "get<$callNbr>(p)";
				} elsif ($parameter =~ /^($dt_char)$/) {
					$functionSignature .= "char, ";
					$functionValues    .= "$1, ";
					$functionName .= "_c";

					$callNbr++;
					$functionNameGet .= "get<$callNbr>(p)";
				} elsif ($parameter =~ /^($dt_str)$/) {
					$functionSignature .= "std::string, ";
					$functionValues    .= "$1, ";
					$functionName .= "_s";

					$callNbr++;
					$functionNameGet .= "get<$callNbr>(p)";

				} else {
					die "error parsing function arguments in file $g_inputFile";
				}
				$functionNameGet .= ", " if $callValue;

			}
			$functionNameGet .= ')';
		}
		if ($function =~ /^(\.|->)(.*)$/) {
			$function = $2;
			$functionNameGet .= $1;
			$functionName    .= "_";
		}
	}

	$functionSignature = $1 if $functionSignature =~ /^(.*), $/;
	$functionValues    = $1 if $functionValues    =~ /^(.*), $/;

	return ($functionSignature, $functionValues, $functionName, $functionNameGet);
}
sub analyseCondition {
	my $condition = shift;

	my $function     = "";
	my $compareFct   = "==";
	my $compareValue = "true";

	unless ($condition =~ /^(?:(!?)($functionRegEx)|($functionRegEx)\s*(==|!=|>|<|>=|<=)\s*($dt_any))\s*$/) {
		die "error in file $g_inputFile";
	}
	$function     = $2      if $2;
	$function     = $3      if $3;
	$compareValue = "false" if $1;
	$compareFct   = $4      if $4;
	$compareValue = $5      if $5;

	my ($functionSignature, $functionValues, $functionName, $functionNameGet) = &analyseFunction($function);

		$functionName .= "_equal"    if $compareFct eq "==";
		$functionName .= "_unequal"  if $compareFct eq "!=";
		$functionName .= "_geater"   if $compareFct eq ">";
		$functionName .= "_lesser"   if $compareFct eq "<";
		$functionName .= "_egreater" if $compareFct eq ">=";
		$functionName .= "_elesser"  if $compareFct eq "<=";

		$functionName .= "_p";

		my $paraNbr = 0;
		if (not ($functionSignature eq "")) {
			$paraNbr++;
			while ($functionSignature =~ m/[,]/g) {
				$paraNbr++;
			}
		}

		if ($compareValue =~ /^$dt_bool$/ or not $compareValue =~ /^$dt_enum$/) {
			$functionNameGet .= " $compareFct get<$paraNbr>(p)";

			$functionValues  .= ", " if $paraNbr;
			$functionValues  .= $compareValue;


			$functionSignature .= ", " if $paraNbr;
			$functionSignature .= "bool"        if $compareValue =~ /^$dt_bool$/;
			$functionSignature .= "int"         if $compareValue =~ /^$dt_int$/;
			$functionSignature .= "double"      if $compareValue =~ /^$dt_float$/;
			$functionSignature .= "char"        if $compareValue =~ /^$dt_char$/;
			$functionSignature .= "std::string" if $compareValue =~ /^$dt_str$/;
		} else {
			$functionNameGet .= " $compareFct $compareValue";

			if ($compareValue =~ /^$dt_enum$/) {
				$compareValue =~ /^(?:$word\:\:)*($word)$/;
				$functionName .= "_$1";
			}
		}

	return ($functionSignature, $functionValues, $functionName, $functionNameGet);
}


sub analyseCode {

	my @inputFileQueue;
	$inputFileQueue[0] = shift;

	my %HoA = ();

	my $fileNbr=0;


	while(scalar @inputFileQueue  > 0) {
		my $inputFile = shift @inputFileQueue;
		my $machineName = "";
		my $stateName   = "";
		open IFILE, "<$inputFile" or die $!;
		$fileNbr = $fileNbr + 1;

		while (<IFILE>) {
			my $line = $_;

			# Extract comments at end of line
			if (/(^[^\#]*)\#.*$/) {
				$line = $1;
			}
			# Process if export keyword
			if ($line =~ /^export\s+($word)\s*$/) {
				if ($fileNbr == 1) {
					@g_exportMachines[scalar @g_exportMachines] = $1;
				}
			# Process includes
			} elsif ($line =~ /^include\s+($word.sm)\s*$/) {
				$inputFileQueue[scalar @inputFileQueue] = $g_dirname."/".$1;
				
			# Process only if not an empty line
			} elsif (not $line =~ /^\s*$/) {

				# Check if new machine name
				if ($line =~ /^($word)$/) {
					$machineName = $1;
					$stateName = "";
					$HoA { $machineName } = { initState => "",
											  stateList => {}};

				} elsif ($line =~ /^\t($word)\s*:\s*(any\s|once\s|min_once\s|max_once\s)?\s*($function)?\s*$/) {
					die "Parsing State without State Machine in file $g_inputFile" if $machineName eq "";
					$stateName = $1;
					$HoA { $machineName }{ initState } = $stateName if $HoA { $machineName }{ initState } eq "";
				
					my $executeType       = "any";
					my $action            = "";

					$executeType = $2 if $2;
					$action      = $3 if $3;
					$executeType = $1 if $executeType =~ /^(.*) $/;
					my ($functionSignature, $functionValues, $functionName, $functionNameGet) = &analyseFunction($action) if $action;
					$HoA { $machineName }{ stateList }{ $stateName }
							= { action => $action,
								functionSignature => $functionSignature,
								functionValues    => $functionValues,
								functionName      => $functionName,
								functionNameGet   => $functionNameGet,
								executeType       => $executeType,
								transitionList    => [],
								submachineList    => [] };

				} elsif ($line =~ /^\t\t($conditionRegEx)\s*->\s*($wordRegEx)\s*$/) {
					die "Parsing transition without state in file $g_inputFile" if $stateName eq "";

					my $condition   = $1;
					my $targetState = $2;

					my ($functionSignature, $functionValues, $functionName, $functionNameGet) = &analyseCondition($condition) if $condition;

					push @{ $HoA { $machineName }{ stateList } { $stateName }{ transitionList }},
							{ condition         => $condition,
							  functionSignature => $functionSignature,
							  functionValues    => $functionValues,
							  functionName      => $functionName,
							  functionNameGet   => $functionNameGet,
							  targetState       => $targetState };
				} elsif ($line =~ /^\t\t\*\s+($word)\s*$/) {
					die "Parsing submachine without state in file $g_inputFile" if $stateName eq "";
					push @{ $HoA { $machineName }{ stateList }{ $stateName }{ submachineList } }, $1;
				} else {
					die "can't parse line $. $_ in $g_inputFile";
				}
			}
		}
		close (IFILE);
	}

	# Check if all target States are valid
	while (my ($m, $mv) = each %HoA) {
		while (my ($s, $sv) = each ${mv}->{stateList}) {
			foreach(@{${sv}->{transitionList}}) {
				my $targetState = $_->{targetState};
				die "Target State: $targetState in machine $m does not exist in file $g_inputFile" if not exists $mv->{stateList}{$targetState};
			}
		}
	}
	return %HoA;
}

sub generateDot {
	my ($inputFile, $outputFile) = (shift, shift);
	my %HoA = &analyseCode($inputFile);

	print OFILE "digraph {$/\tnode [ fontsize=13 ];$/\tedge [ fondsize=12 ];$/";
	while (my ($m, $mv) = each %HoA) {
		print OFILE "\tsubgraph cluster$m {$/";
		print OFILE "\t\tgraph [label=\"$m\"];$/";
		print OFILE "\t\tm_${m}_init [label=\"\", shape=point, styele=filled, fillcolor=\"#000000\"];$/";
		print OFILE "\t\tm_${m}_init-> m_${m}_s_".${mv}->{initState}.";$/$/";
		while (my ($s, $sv) = each ${mv}->{stateList}) {
			my $label = $s;
			$label = "$label\\n<".${sv}->{executeType}.">" unless ${sv}->{executeType} eq "any";
			my $action = ${sv}->{action};
			$action =~ s/\"/\\\"/g;
			$label = "$label\\n".$action unless $action eq "";
			print OFILE "\t\tm_${m}_s_${s} [label=\"$label\" style=rounded, shape=box];$/";
		}
		print OFILE "$/$/";

		while (my ($s, $sv) = each ${mv}->{stateList}) {
			foreach(@{${sv}->{transitionList}}) {
				my $condition = $_->{condition};
				$condition =~ s/\"/\\\"/g;
				print OFILE "\t\tm_${m}_s_${s} -> m_${m}_s_".$_->{targetState}." [label=\"$condition\"];$/";
			}
		}
		print OFILE "$/$/";

		while (my ($s, $sv) = each ${mv}->{stateList}) {
			foreach(@{${sv}->{submachineList}}) {
				print OFILE "\t\tm_${m}_cluster${_} [label=\"${_}\", shape=box];$/";
			}
		}
		print OFILE "$/$/";
		while (my ($s, $sv) = each ${mv}->{stateList}) {
			foreach(@{${sv}->{submachineList}}) {
				print OFILE "\t\tm_${m}_s_${s} -> m_${m}_cluster${_} [style=dashed];$/";
			}
		}
		print OFILE "\t}$/$/";

	}
	print OFILE "}$/";

}
sub printTransition {
	my ($srcState, $targetState, $conditionName, $conditionSignature, $conditionValues) = (shift, shift, shift, shift, shift);

	print OFILE "	{$/";
	print OFILE "		auto conditionIter = stateConditionMap.find(\"${srcState}_${conditionName}\");$/";
	print OFILE "		if (conditionIter == stateConditionMap.end()) conditionIter = conditionMap.find(\"$conditionName\");$/";
	print OFILE "		else if (neededMethods.find(\"$conditionName\") != neededMethods.end()) neededMethods.erase(neededMethods.find(\"$conditionName\"));$/";
	print OFILE "		if (conditionIter == conditionMap.end()) { conditionIter = conditionMap.find(\"false_equal_p\"); notFoundMethods.insert(\"${conditionName}\"); }$/";
	print OFILE "		Parameter_Impl<$conditionSignature> parameter(std::make_tuple($conditionValues));$/";
	print OFILE "		states.at(\"$srcState\")->addTransition(conditionIter->second(&parameter), states.at(\"$_->{targetState}\"));$/";

	print OFILE "	}$/";

}
sub printState {
	my ($HoA, $machineName, $stateName) = (shift, shift, shift);

	my $state = $HoA->{$machineName}{stateList}{$stateName};

	my $executeType = "State::ExecuteType::Any";
	$executeType = "State::ExecuteType::Once"    if $state->{executeType} eq "once";
	$executeType = "State::ExecuteType::MaxOnce" if $state->{executeType} eq "max_once";
	$executeType = "State::ExecuteType::MinOnce" if $state->{executeType} eq "min_once";

	my $functionName      = "";
	my $functionValues    = "";
	my $functionSignature = "";

	$functionName      = $state->{functionName}      if $state->{functionName};
	$functionValues    = $state->{functionValues}    if $state->{functionValues};
	$functionSignature = $state->{functionSignature} if $state->{functionSignature};
	my $action = "actionMap.at((actionMap.find(\"$functionName\")!=actionMap.end())?\"$functionName\":\"\")(&parameter)";

	print OFILE "	{$/";
	print OFILE "	MachinePtrList& superMachines = machines;$/";
	print OFILE "	std::string& superStateName = stateName;$/";
	print OFILE "	{$/";
	print OFILE "		std::string stateName = \"$stateName\";$/";
	print OFILE "		MachinePtrList machines;$/";
	if(@{$state->{submachineList}}) {
		print OFILE "		{$/";
		print OFILE "			ConditionParaMap& superStateConditionMap = stateConditionMap;$/";
		foreach(@{$state->{submachineList}}) {
			&printMachine($HoA, $_);
		}
		print OFILE "		}$/";
	}
	print OFILE "		Parameter_Impl<$functionSignature> parameter(std::make_tuple($functionValues));$/";
	print OFILE "		states[\"$stateName\"] = new State($executeType, $action, machines);$/";
	print OFILE "		Machine* m = superMachines[superMachines.size()-1].get();$/";
	print OFILE "		State*   s = states[\"$stateName\"];$/";
	print OFILE "		superStateConditionMap[superStateName+\"_${machineName}_${stateName}_equal_p\"] = [m, s](Parameter const* _p) {$/";
	print OFILE "			auto p = dynamic_cast<Parameter_Impl<bool> const*>(_p);$/";
	print OFILE "			auto t = p->tuple;$/";
	print OFILE "			return [m, s, t]() { return (m->getCurrentState() == s) == std::get<0>(t); };$/";
	print OFILE "		};$/";
	print OFILE "	}}$/";
}
sub printMachine {
	my ($HoA, $machineName) = (shift, shift);
	print OFILE "	{ std::map<std::string, State*> states;$/";
	print OFILE "	machines.push_back(std::move(std::unique_ptr<Machine>(new Machine)));$/";
	print OFILE "	ConditionParaMap stateConditionMap;$/";

	while (my ($s, $sv) = each $HoA->{$machineName}{stateList}) {
		&printState($HoA, $machineName, $s);
	}
	while (my ($s, $sv) = each $HoA->{$machineName}{stateList}) {
		foreach(@{${sv}->{transitionList}}) {
			&printTransition($s, $_->{targetState}, $_->{functionName}, $_->{functionSignature}, $_->{functionValues});
		}
	}
	my $initState = $HoA->{$machineName}{initState};
	print OFILE "	machines[machines.size()-1]->setInitState(states.at(\"$initState\"));$/";
	print OFILE "	}$/";
}
sub generateCpp11 {

	my ($inputFile, $outputFile, $_HoA) = (shift, shift, shift);
	my %HoA = %{$_HoA};

	my %actions    = ();
	my %conditions = ();

	while (my ($m, $mv) = each %HoA) {
		while (my ($s, $sv) = each ${mv}->{stateList}) {
			if ($sv->{functionName}) {
			$actions{$sv->{functionName}} = { call      => $sv->{functionNameGet},
			                                  signature => $sv->{functionSignature}};
			}

			foreach(@{$sv->{transitionList}}) {
				next unless $_->{functionName};
				next if $_->{functionName} eq "true_equal_p";
				next if $_->{functionName} eq "true_unequal_p";
				next if $_->{functionName} eq "false_equal_p";
				next if $_->{functionName} eq "false_unequal_p";
				next if $_->{functionName} eq "else_equal_p";

				$conditions{$_->{functionName}} = { call      => $_->{functionNameGet},
				                                    signature => $_->{functionSignature}};
			}
		}
	}

	foreach (@g_exportMachines) {
		my $Machine = $_;
		my $MACHINE = uc $Machine;
		print OFILE "#ifndef SIMPLESTATEMACHINE_GENERATED_${MACHINE}$/";
		print OFILE "#define SIMPLESTATEMACHINE_GENERATED_${MACHINE}$/";
		print OFILE "$/";
		print OFILE "#include $g_includeSSM$/";
		print OFILE "using namespace SimpleStateMachine;$/";
		print OFILE "class ${Machine} : public IStateMachine {$/";
		print OFILE "private:$/";
		print OFILE "	ActionParaMap actionMap;$/";
		print OFILE "	ConditionParaMap conditionMap;$/";
		print OFILE "	std::unique_ptr<Machine> machine;$/";
		print OFILE "	std::set<std::string> neededMethods;$/";
		print OFILE "   std::set<std::string> notFoundMethods;$/";
		print OFILE "	SimpleStateMachine::Timer timer;$/";
		print OFILE "	friend struct CallMethod;$/";
		print OFILE "public:$/";
		print OFILE "	virtual ~${Machine}() override {}$/";
		print OFILE "	struct MethodCall;$/";
		print OFILE "	template<typename ...Args>$/";
		print OFILE "	${Machine}(std::tuple<Args...> _t)$/";
		print OFILE "		: IStateMachine(\"${Machine}\") {$/";
		print OFILE "		initNeededMethods();$/";
		print OFILE "		auto t = std::tuple_cat(std::make_tuple(&timer), _t);$/";
		print OFILE "		${Machine}autoRegisterAll(&actionMap, &conditionMap, t, neededMethods);$/";
		print OFILE "		machine = std::move(get(actionMap, conditionMap));$/";
		print OFILE "		machine->start();$/";
		print OFILE "	}$/";
		print OFILE "	template<typename T>$/";
		print OFILE "	${Machine}(T* _o)$/";
		print OFILE "		: IStateMachine(\"${Machine}\") {$/";
		print OFILE "		initNeededMethods();$/";
		print OFILE "		auto t = std::make_tuple(&timer, _o);$/";
		print OFILE "		${Machine}autoRegisterAll(&actionMap, &conditionMap, t, neededMethods);$/";
		print OFILE "		machine = std::move(get(actionMap, conditionMap));$/";
		print OFILE "		machine->start();$/";
		print OFILE "	}$/";
		print OFILE "	std::set<std::string> const& getUnmatchedSymbols() const override { return notFoundMethods; }$/";
		print OFILE "	bool step() override {$/";
		print OFILE "		machine->step();$/";
		print OFILE "		return machine->hasTransitions();$/";
		print OFILE "	}$/";
		print OFILE "private:$/";
		print OFILE "	std::unique_ptr<Machine> get(ActionParaMap actionMap, ConditionParaMap conditionMap) {$/";
		print OFILE "		MachinePtrList machines;$/";
		print OFILE "		ConditionParaMap stateConditionMap;$/";
		print OFILE "		ConditionParaMap superStateConditionMap;$/";
		print OFILE "		std::string stateName = \"\";$/";
			&printMachine(\%HoA, $Machine);
		print OFILE "		return std::move(machines[0]);$/";
		print OFILE "	}$/";
		print OFILE "	void initNeededMethods() {$/";
		print OFILE "		neededMethods = {$/";
			while (my ($a, $av) = each %actions) {
				print OFILE "\"$a\",$/";
			}
			while (my ($a, $av) = each %conditions) {
				print OFILE "\"$a\",$/";
			}
		print OFILE "	};$/";
		print OFILE "	}$/";
		print OFILE "$/";
		print OFILE "};$/";


		print OFILE "DEF_GET_METHOD_CALL_BEGIN(${Machine})$/";
		while (my ($a, $av) = each %actions) {
			print OFILE "\tDEF_GET_METHOD_CALL($a, $av->{call}, void";
			print OFILE ", $av->{signature}" if $av->{signature};
			print OFILE ")$/";
		}
		while (my ($a, $av) = each %conditions) {
			print OFILE "\tDEF_GET_METHOD_CALL($a, $av->{call}, bool";
			print OFILE ", $av->{signature}" if $av->{signature};
			print OFILE ")$/";
		}
		print OFILE "DEF_GET_METHOD_CALL_END$/";

		print OFILE "DEF_AUTO_REGISTER_BEGIN(${Machine})$/";

		while (my ($a, $av) = each %actions) {
			print OFILE "	DEF_AUTO_REGISTER_ACTION(${Machine}, $a)$/";
		}
		while (my ($a, $av) = each %conditions) {
			print OFILE "	DEF_AUTO_REGISTER_CONDITION(${Machine}, $a)$/";
		}
		print OFILE "DEF_AUTO_REGISTER_END(${Machine})$/";

		print OFILE "#endif$/";
	}

	print OFILE "#ifndef SIMPLESTATEMACHINE_FACTORY$/";
	print OFILE "#define SIMPLESTATEMACHINE_FACTORY$/";
	print OFILE "namespace SimpleStateMachine {$/";
	print OFILE "$/";
	print OFILE "class Factory {$/";
	print OFILE "public:$/";
	print OFILE "	template<typename T>$/";
	print OFILE "	static std::unique_ptr<IStateMachine> createStateMachine(std::string const& str, T t) {$/";
		foreach (@g_exportMachines) {
			print OFILE "		if (str == \"$_\") return std::unique_ptr<IStateMachine>(new $_(t));$/";
		}
	print OFILE "		return nullptr;$/";
	print OFILE "	}$/";
	print OFILE "	static std::set<std::string> getStateMachineNames() {$/";
	print OFILE "		std::set<std::string> stateMachineNames {";
	foreach (@g_exportMachines) {
		print OFILE "\"$_\", ";
	}
	print OFILE "};$/";
	print OFILE "		return stateMachineNames;$/";
	print OFILE "	}$/";
	print OFILE "};$/";
	print OFILE "}$/";
	print OFILE "#endif$/";

}
sub printHelp
{
	print "Usage:$/ssm.pl [--cpp11] [--dot] [--png] [--view] <file>$/";
	print "creates a diagram to view *.sm files$/$/";
	print "--cpp11                 creates header file to include by your project$/$/";
	print "--export                list all machines that would be exported$/$/";
	print "--dot                   creates a .dot file that can be parsed by graphviz$/$/";
	print "--png                   creates a .png file of the given state machine$/$/";
	print "--view                  opens eog to view given state machine$/$/";

}


my $dot    = 0;
my $png    = 0;
my $help   = 0;
my $cpp11  = 0;
my $view   = 0;
my $export = 0;

$help = 1 unless GetOptions ("dot"       => \$dot,
                             "cpp11"     => \$cpp11,
                             "png"       => \$png,
                             "view"      => \$view,
                             "export"    => \$export,
                             "ssm-include|i=s" => \$g_includeSSM,
                             "help|h"    => \$help);
my $file = shift;

$g_dirname = dirname($file);

$help = 1 unless $file;
#$help = 1 unless defined $input;
if ($help) {
	&printHelp;
	exit;
}
my $inputFile;
my $outputFile;

if (not $file =~ /^(.*\.sm)$/) {
	die "wrong file ending$/";
}
$inputFile = "$1";
$g_inputFile = $inputFile;

if ($dot) {
	$outputFile = "$inputFile.dot";
	open OFILE, ">$outputFile" or die $!;

	&generateDot($inputFile, $outputFile);
} elsif ($png) {
	$outputFile = "$inputFile.png";
	open OFILE, "| dot -Tpng >$outputFile" or die $!;

	&generateDot($inputFile, $outputFile);
} elsif ($view) {
	$outputFile = "/tmp/ssm_tmp.png";
	open OFILE, "| dot -Tpng >$outputFile; eog $outputFile" or die $!;

	&generateDot($inputFile, $outputFile);
} elsif ($cpp11) {
	$outputFile = "$inputFile.h";
	my %HoA = &analyseCode($inputFile);

	if (scalar @g_exportMachines > 0) {
		open OFILE, ">$outputFile" or die $!;

		&generateCpp11($inputFile, $outputFile, \%HoA);
	}
} elsif ($export) {
	my %HoA = &analyseCode($inputFile);
	foreach (@g_exportMachines) {
		print $_.$/;
	}
} else {
	&printHelp;
}

