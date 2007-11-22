/*******************************************************************
 * MOPSLinux packaging system
 * Package builder
 * $Id: mainwindow.cpp,v 1.51 2007/11/22 15:32:56 i27249 Exp $
 * ***************************************************************/

#include "mainwindow.h"



Form::Form(QWidget *parent, string arg)
{
	pBuilder_isStartup = true;
	if (arg.empty()) dataType=DATATYPE_NEW;
	_arg = arg;
	modified=false;
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf-8"));
	short_description.resize(2);
	description.resize(2);
	packageDir = new QDir;
	packageDir->setSorting(QDir::DirsFirst | QDir::IgnoreCase);
	currentPackageDir = new QDir;
	currentPackageDir->setSorting(QDir::DirsFirst | QDir::IgnoreCase);

	currentFilesystemDir = new QDir(QDir::currentPath());
	currentFilesystemDir->setSorting(QDir::DirsFirst | QDir::IgnoreCase);

	if (parent==0) ui.setupUi(this);
	else ui.setupUi(parent);

	debugLabel = new QLabel;
	debugLabel->setText("Debug window");
	ui.MaintainerNameEdit->setText(mConfig.getValue("maintainer_name").c_str());
	ui.MaintainerMailEdit->setText(mConfig.getValue("maintainer_email").c_str());

	ui.openDirectoryButton->hide();	
	//ui.DepTableWidget->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
	//ui.DepTableWidget->horizontalHeader()->hide();
	ui.DepTableWidget->verticalHeader()->hide();
	connect(ui.saveAndQuitButton, SIGNAL(clicked()), this, SLOT(saveAndExit()));
	connect(ui.addDepFromFilesBtn, SIGNAL(clicked()), this, SLOT(addDepsFromFiles()));

	connect(ui.aboutButton, SIGNAL(clicked()), this, SLOT(showAbout()));
	connect(ui.browsePatchButton, SIGNAL(clicked()), this, SLOT(browsePatch()));
// Build options UI switches

	connect(ui.editWithGvimButton, SIGNAL(clicked()), this, SLOT(editBuildScriptWithGvim()));
	connect(ui.loadScriptFromFileButton, SIGNAL(clicked()), this, SLOT(loadBuildScriptFromFile()));
	connect(ui.envOptionsCheckBox, SIGNAL(stateChanged(int)), this, SLOT(switchEnvField(int)));
	switchEnvField(ui.envOptionsCheckBox->checkState());
	connect(ui.configureCheckBox, SIGNAL(stateChanged(int)), this, SLOT(switchConfigureField(int)));
	switchConfigureField(ui.configureCheckBox->checkState());
	connect(ui.compilationCheckBox, SIGNAL(stateChanged(int)), this, SLOT(switchCompilationField(int)));
	switchCompilationField(ui.compilationCheckBox->checkState());
	connect(ui.installationCheckBox, SIGNAL(stateChanged(int)), this, SLOT(switchInstallField(int)));
	switchInstallField(ui.installationCheckBox->checkState());
	connect(ui.sourcesRootDirectoryAutodetectCheckBox, SIGNAL(stateChanged(int)), this, SLOT(switchSourceDirectoryField(int)));
	switchSourceDirectoryField(ui.sourcesRootDirectoryAutodetectCheckBox->checkState());
	connect(ui.buildingSystemComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(switchBuildSystem(int)));
	switchBuildSystem(ui.buildingSystemComboBox->currentIndex());
	connect(ui.cpuArchCheckBox, SIGNAL(stateChanged(int)), this, SLOT(switchCpuArchField(int)));
	switchCpuArchField(ui.cpuArchCheckBox->checkState());
	connect(ui.cpuTuneCheckBox, SIGNAL(stateChanged(int)), this, SLOT(switchCpuTuneField(int)));
	switchCpuTuneField(ui.cpuTuneCheckBox->checkState());
	connect(ui.optimizationCheckBox, SIGNAL(stateChanged(int)), this, SLOT(switchOptimizationField(int)));
	switchOptimizationField(ui.optimizationCheckBox->checkState());
	connect(ui.customGccOptionsCheckBox, SIGNAL(stateChanged(int)), this, SLOT(switchGccOptionsField(int)));
	switchGccOptionsField(ui.customGccOptionsCheckBox->checkState());


	connect(ui.patchAddButton, SIGNAL(clicked()), this, SLOT(addPatch()));
	connect(ui.keyAddButton, SIGNAL(clicked()), this, SLOT(addKey()));
	connect(ui.patchDeleteButton, SIGNAL(clicked()), this, SLOT(deletePatch()));
	connect(ui.keyDeleteButton, SIGNAL(clicked()), this, SLOT(deleteKey()));

	connect(ui.downloadAnalyzeButton, SIGNAL(clicked()), this, SLOT(analyzeSources()));

focusIndex=0;
	//connect(ui.filelistWidget, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(setNewPackageCurrentDirectory(QListWidgetItem *)));
	connect(ui.filelistWidget, SIGNAL(itemActivated(QListWidgetItem *)), this, SLOT(setNewPackageCurrentDirectory(QListWidgetItem *)));
	connect(ui.filelistWidget_2, SIGNAL(itemActivated(QListWidgetItem *)), this, SLOT(setNewFilesystemCurrentDirectory(QListWidgetItem *)));

	connect(ui.goHomeButton, SIGNAL(clicked()), this, SLOT(goPackageHome()));
	connect(ui.goHomeButton_2, SIGNAL(clicked()), this, SLOT(goFilesystemHome()));
	connect(ui.goUpButton, SIGNAL(clicked()), this, SLOT(goPackageUp()));
	connect(ui.goUpButton_2, SIGNAL(clicked()), this, SLOT(goFilesystemUp()));

	connect(ui.copyButton, SIGNAL(clicked()), this, SLOT(copyFiles()));
	connect(ui.moveButton, SIGNAL(clicked()), this, SLOT(moveFiles()));
	connect(ui.deleteButton, SIGNAL(clicked()), this, SLOT(removeFiles()));
	connect(ui.renameButton, SIGNAL(clicked()), this, SLOT(renameFile()));
	connect(ui.viewButton, SIGNAL(clicked()), this, SLOT(viewFile()));
	connect(ui.editButton, SIGNAL(clicked()), this, SLOT(editFile()));
	connect(ui.mkdirButton, SIGNAL(clicked()), this, SLOT(makeDirectory()));
	connect(ui.filelistWidget, SIGNAL(iGotFocus()), this, SLOT(setPanel1Focus()));
	connect(ui.filelistWidget_2, SIGNAL(iGotFocus()), this, SLOT(setPanel2Focus()));
	connect(ui.refreshButton, SIGNAL(clicked()), this, SLOT(reloadPackageDirListing()));
	connect(ui.refreshButton_2, SIGNAL(clicked()), this, SLOT(reloadFilesystemDirListing()));
	connect(ui.cmdLine, SIGNAL(execCmd()), this, SLOT(execShellCommand()));
	
	ui.cmdLine->addItem("");
	reloadFilesystemDirListing();
	ui.cmdLine->setEnabled(false);
	this->showMaximized();
	ui.filelistCurrentPath->setText(tr("Package should be saved before using this tool"));
	//ui.filelistCurrentPath_2->setText(currentFilesystemDir->canonicalPath());


	loadFile(arg.c_str());
}
void Form::execShellCommand()
{
	if (ui.cmdLine->currentText().isEmpty()) return;
	QDir *curr;
	switch(focusIndex) {
		case 1:
			curr = currentPackageDir;
			break;
		case 2:
			curr = currentFilesystemDir;
			break;
		default: return;
	}
	QString cmd = "(cd " + curr->absolutePath() + "; " + ui.cmdLine->currentText() + ")";
	system(cmd.toStdString());
	reloadFilesystemDirListing();
	reloadPackageDirListing();
	ui.cmdLine->insertItem(1,cmd);
}
void Form::setCurrentPathLabel()
{
	if (focusIndex==1) ui.currentPath->setText("["+currentPackageDir->absolutePath()+"]# ");
	else ui.currentPath->setText("["+currentFilesystemDir->absolutePath()+"]# ");
}

void Form::setPanel1Focus()
{
	reloadPackageDirListing();
	printf("Got panel 1 focus\n");
	focusIndex = 1;
	setCurrentPathLabel();
}
void Form::setPanel2Focus()
{
	reloadFilesystemDirListing();
	printf("Got panel 2 focus\n");
	focusIndex = 2;
	setCurrentPathLabel();
}


void Form::embedSources()
{
	if (!sourcePackage.isSourceEmbedded(ui.urlEdit->text().toStdString()))
	{
		sourcePackage.embedSource(ui.urlEdit->text().toStdString());
	}
	
}
void Form::embedPatches()
{
	for (unsigned int i=0; i<patchList.size(); i++)
	{
		sourcePackage.embedPatch(patchList[i]);
	}
}

void Form::analyzeSources()
{
	SourceFile sFile;
	sFile.setUrl(ui.urlEdit->text().toStdString());
	sFile.download();
	string configHelp;
	sFile.analyze(&configHelp);
	ui.buildingSystemComboBox->setCurrentIndex(sFile.getBuildType());
	if (sFile.getBuildType()==0) {
		QTextBrowser *browser = new QTextBrowser;
		browser->setText(configHelp.c_str());
		browser->showMaximized();
	}
	switchBuildSystem(ui.buildingSystemComboBox->currentIndex());
}

string cleanDescr(string str)
{
	string ret;
	if (str.find_first_of("\n")==std::string::npos) return str;
	while (str.find_first_of("\n")!=std::string::npos)
	{
		ret += str.substr(0,str.find_first_of("\n"));
		if (str.find_first_of("\n")<str.length()-1) str=str.substr(str.find_first_of("\n")+1);
		else {
			ret+=str;
			str.clear();
		}
	}
	ret+=str;
	return ret;
}
void Form::loadData()
{
	int ret;
	if (modified) {

		ret = (QMessageBox::warning(this, tr("MOPSLinux package builder"),
                   tr("The document has been modified.\n"
                      "Do you want to save your changes?"),
                   QMessageBox::Save | QMessageBox::Discard
                   | QMessageBox::Cancel,
                   QMessageBox::Save));
		if (ret == QMessageBox::Save) saveFile();
	}

	loadFile("");
}

	
void Form::loadFile(QString filename)
{
	reloadFilesystemDirListing();
	reloadPackageDirListing();

	if (filename.isEmpty() && pBuilder_isStartup) {
		pBuilder_isStartup=false;
		return;
	}
	if (filename.isEmpty() && !pBuilder_isStartup)
	{
		filename = QFileDialog::getOpenFileName(this, tr("Open a package (.tgz or .spkg)"), "");
		if (filename.isEmpty()) return;
	}

	dataType = DATATYPE_UNKNOWN;
	if (getExtension(filename.toStdString())=="tgz") dataType = DATATYPE_BINARYPACKAGE;
	if (getExtension(filename.toStdString())=="spkg") dataType = DATATYPE_SOURCEPACKAGE;
	if (getExtension(filename.toStdString())=="xml") dataType = DATATYPE_XML;
	if (isDirectory(filename.toStdString())) dataType = DATATYPE_DIR;
	if (dataType == DATATYPE_DIR)
	{
		if (QMessageBox::question(this, tr("Cannot determine tree type"), tr("Is it a binary package tree?"), 
					QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes)==QMessageBox::Yes)
		{
			binaryPackage.importDirectory(filename.toStdString());
			dataType = DATATYPE_BINARYPACKAGE;
		}
		else {
			sourcePackage.importDirectory(filename.toStdString());
			dataType = DATATYPE_SOURCEPACKAGE;
		}
	}
	PackageConfig *p;
	switch(dataType)
	{
		case DATATYPE_BINARYPACKAGE:
			if (!binaryPackage.setInputFile(filename.toStdString())) {
				mError("Cannot open file"); // TODO: Replace to GUI error message
				return;
			}
			if (!binaryPackage.unpackFile()) {
				mError("Error while opening file: cannot extract");
				return;
			}
			//ui.directoryPath->setText(binaryPackage.pkg_dir.c_str());
			p = new PackageConfig(binaryPackage.getDataFilename());
			if (!p->parseOk) {
				mError("Error parsing XML data");
				return;
			}
			ui.sourcePackageRadioButton->setChecked(false);
			ui.binaryPackageRadioButton->setChecked(true);
			ui.packageTypeGroupBox->setEnabled(false);
			break;

		case DATATYPE_SOURCEPACKAGE:
			if (!sourcePackage.setInputFile(filename.toStdString())) {
				mError("Cannot open file");
				return;
			}
			if (!sourcePackage.unpackFile()) {
				mError("Error while opening file: cannot extract");
				return;
			}
			//ui.directoryPath->setText(sourcePackage.pkg_dir.c_str());

			p = new PackageConfig(sourcePackage.getDataFilename());
			if (!p->parseOk) {
				mError("Error parsing XML data");
				return;
			}
			ui.sourcePackageRadioButton->setChecked(true);
			ui.binaryPackageRadioButton->setChecked(false);
			ui.packageTypeGroupBox->setEnabled(false);

			break;
		case DATATYPE_XML:
			p = new PackageConfig(filename.toStdString());
			if (!p->parseOk) {
				mError("Error parsing XML data");
				return;
			}
			break;
		default:
			mError("Unknown data type");
			return;

	}

	// Loading sources info
	ui.urlEdit->setText(p->getBuildUrl().c_str());
	if (dataType==DATATYPE_SOURCEPACKAGE) {
		if (sourcePackage.isSourceEmbedded(ui.urlEdit->text().toStdString())) ui.embedSourcesCheckBox->setCheckState(Qt::Checked);
		else ui.embedSourcesCheckBox->setCheckState(Qt::Unchecked);
	}
	if (p->getBuildNoSubfolder()) ui.noSubfolderCheckBox->setCheckState(Qt::Checked);
	else ui.noSubfolderCheckBox->setCheckState(Qt::Unchecked);
	if (p->getBuildSourceRoot().empty()) {
		ui.sourcesRootDirectoryAutodetectCheckBox->setCheckState(Qt::Checked);
		ui.sourcesRootDirectoryEdit->setText("");
	}
	else {
		ui.sourcesRootDirectoryAutodetectCheckBox->setCheckState(Qt::Unchecked);
		ui.sourcesRootDirectoryEdit->setText(p->getBuildSourceRoot().c_str());
	}
	patchList = p->getBuildPatchList();
	displayPatches();
// Directory listing
	
	QString path;
	switch(dataType) {
		case DATATYPE_NEW: debugLabel->setText("New package, no path still");
				   return;
				   break;
		case DATATYPE_SOURCEPACKAGE:
				   path = QString::fromStdString(sourcePackage.pkg_dir);
				   debugLabel->setText("Source package, path: " + path);
				   break;
		case DATATYPE_BINARYPACKAGE:
				   path = QString::fromStdString(binaryPackage.pkg_dir);

				   debugLabel->setText("Binary package, path: " + path);
				   break;
		default:
				   debugLabel->setText("Unknown/no datatype");
	}
	ui.filelistWidget->clear();
	packageDir->cd(path);
	currentPackageDir->cd(path);
	reloadPackageDirListing();

	// stub for script path
	if (p->getBuildSystem()=="autotools") ui.buildingSystemComboBox->setCurrentIndex(0);
	if (p->getBuildSystem()=="scons") ui.buildingSystemComboBox->setCurrentIndex(1);
	if (p->getBuildSystem()=="cmake") ui.buildingSystemComboBox->setCurrentIndex(2);
	if (p->getBuildSystem()=="custom") ui.buildingSystemComboBox->setCurrentIndex(3);
	if (p->getBuildSystem()=="script") {
		ui.buildingSystemComboBox->setCurrentIndex(4);
		ui.customScriptTextEdit->setText(sourcePackage.readBuildScript().c_str());
	}
	switchBuildSystem(ui.buildingSystemComboBox->currentIndex());

	if (p->getBuildConfigureEnvOptions().empty())
	{
		ui.envOptionsEdit->clear();
		ui.envOptionsCheckBox->setCheckState(Qt::Unchecked);
	}
	else
	{
		ui.envOptionsCheckBox->setCheckState(Qt::Checked);
		ui.envOptionsEdit->setText(p->getBuildConfigureEnvOptions().c_str());
	}


	if (p->getBuildOptimizationMarch().empty())
	{
		ui.cpuArchCheckBox->setCheckState(Qt::Unchecked);
	}
	else 
	{
		ui.cpuArchCheckBox->setCheckState(Qt::Checked);
		if (ui.cpuArchComboBox->findText(p->getBuildOptimizationMarch().c_str())<0) ui.cpuArchComboBox->addItem(p->getBuildOptimizationMarch().c_str());
		ui.cpuArchComboBox->setCurrentIndex(ui.cpuArchComboBox->findText(p->getBuildOptimizationMarch().c_str()));
	}
	if (p->getBuildOptimizationMtune().empty())
	{
		ui.cpuTuneCheckBox->setCheckState(Qt::Unchecked);
	}
	else 
	{
		ui.cpuTuneCheckBox->setCheckState(Qt::Checked);
		if (ui.cpuTuneComboBox->findText(p->getBuildOptimizationMtune().c_str())<0) ui.cpuTuneComboBox->addItem(p->getBuildOptimizationMtune().c_str());
		ui.cpuTuneComboBox->setCurrentIndex(ui.cpuTuneComboBox->findText(p->getBuildOptimizationMtune().c_str()));
	}
	if (p->getBuildOptimizationLevel().empty())
	{
		ui.optimizationCheckBox->setCheckState(Qt::Unchecked);
	}
	else 
	{
		ui.optimizationCheckBox->setCheckState(Qt::Checked);
		if (ui.optimizationComboBox->findText(p->getBuildOptimizationLevel().c_str())<0) ui.optimizationComboBox->addItem(p->getBuildOptimizationLevel().c_str());
		ui.optimizationComboBox->setCurrentIndex(ui.optimizationComboBox->findText(p->getBuildOptimizationLevel().c_str()));
	}
	if (p->getBuildOptimizationCustomGccOptions().empty()) 
	{
		ui.customGccOptionsCheckBox->setCheckState(Qt::Unchecked);
		ui.customGccOptionsEdit->setText("");
	}
	else {
		ui.customGccOptionsCheckBox->setCheckState(Qt::Checked);
		ui.customGccOptionsEdit->setText(p->getBuildOptimizationCustomGccOptions().c_str());
	}
		if (p->getBuildCmdConfigure().empty()) {
		ui.configureCheckBox->setCheckState(Qt::Unchecked);
		ui.configureEdit->setText("");
	}
	else {
		ui.configureCheckBox->setCheckState(Qt::Checked);
		ui.configureEdit->setText(p->getBuildCmdConfigure().c_str());
	}
	if (p->getBuildCmdMake().empty()) {
		ui.compilationCheckBox->setCheckState(Qt::Unchecked);
		ui.compilationEdit->setText("");
	}
	else {
		ui.compilationCheckBox->setCheckState(Qt::Checked);
		ui.compilationEdit->setText(p->getBuildCmdMake().c_str());
	}
	if (p->getBuildCmdMakeInstall().empty()) {
		ui.installationCheckBox->setCheckState(Qt::Unchecked);
		ui.installEdit->setText("");
	}
	else {
		ui.installationCheckBox->setCheckState(Qt::Checked);
		ui.installEdit->setText(p->getBuildCmdMakeInstall().c_str());
	}
	if (p->getBuildOptimizationCustomizable()) ui.AllowUserToChangeCheckBox->setCheckState(Qt::Checked);
	else ui.AllowUserToChangeCheckBox->setCheckState(Qt::Unchecked);

	if (p->getBuildUseCflags()) ui.dontUseCflags->setCheckState(Qt::Unchecked);
	else ui.dontUseCflags->setCheckState(Qt::Checked);

	ui.maxNumjobsSpinBox->setValue(atoi(p->getBuildMaxNumjobs().c_str()));
	keyList.clear();
	keys key_tmp;
	vector<string> key_names = p->getBuildKeyNames();
	vector<string> key_values = p->getBuildKeyValues();
	for (unsigned int i=0; i<key_names.size(); i++)
	{
		key_tmp.name=key_names[i];
		key_tmp.value=key_values[i];
		keyList.push_back(key_tmp);
	}
	displayKeys();

	// Loading scripts info
	switch(dataType) {
		case DATATYPE_BINARYPACKAGE:
			ui.preinstallScriptEdit->setText(binaryPackage.readPreinstallScript().c_str());
			ui.postinstallScriptEdit->setText(binaryPackage.readPostinstallScript().c_str());
			ui.preremoveScriptEdit->setText(binaryPackage.readPreremoveScript().c_str());
			ui.postremoveScriptEdit->setText(binaryPackage.readPostremoveScript().c_str());
			break;
		case DATATYPE_SOURCEPACKAGE:
			ui.preinstallScriptEdit->setText(sourcePackage.readPostinstallScript().c_str());
			ui.postinstallScriptEdit->setText(binaryPackage.readPostinstallScript().c_str());
			ui.preremoveScriptEdit->setText(binaryPackage.readPreremoveScript().c_str());
			ui.postremoveScriptEdit->setText(binaryPackage.readPostremoveScript().c_str());
			break;
		default:
			mWarning("This data type doesn't support scripts");
	}
	
	// Loading main package data
	PACKAGE pkg;
	xml2package(p->getXMLNode(), &pkg);
	pkg.sync();

	// Filling data 
	ui.NameEdit->setText(pkg.get_name()->c_str());
	ui.VersionEdit->setText(pkg.get_version()->c_str());
	ui.betaReleaseEdit->setText(pkg.get_betarelease()->c_str());
	ui.ArchComboBox->setCurrentIndex(ui.ArchComboBox->findText(pkg.get_arch()->c_str()));
	ui.BuildEdit->setText(pkg.get_build()->c_str());
	ui.ShortDescriptionEdit->setText(pkg.get_short_description()->c_str());
	short_description[0]=pkg.get_short_description()->c_str();
	description[0]=cleanDescr(*pkg.get_description()).c_str();
	ui.ShortDescriptionEdit->setText(short_description[0]);
	ui.DescriptionEdit->setText(description[0]);
	ui.ChangelogEdit->setText(pkg.get_changelog()->c_str());
	ui.MaintainerNameEdit->setText(pkg.get_packager()->c_str());
	ui.MaintainerMailEdit->setText(pkg.get_packager_email()->c_str());

	for (unsigned int i=0; i<pkg.get_dependencies()->size(); i++)
	{
		ui.DepTableWidget->insertRow(0);
		ui.DepTableWidget->setItem(0,3, new QTableWidgetItem(pkg.get_dependencies()->at(i).get_type()->c_str()));
		ui.DepTableWidget->setItem(0,0, new QTableWidgetItem(pkg.get_dependencies()->at(i).get_package_name()->c_str()));
		ui.DepTableWidget->setItem(0,1, new QTableWidgetItem(pkg.get_dependencies()->at(i).get_vcondition().c_str()));
		ui.DepTableWidget->setItem(0,2, new QTableWidgetItem(pkg.get_dependencies()->at(i).get_package_version()->c_str()));
		if (pkg.get_dependencies()->at(i).isBuildOnly()) ui.DepTableWidget->setItem(0,4,new QTableWidgetItem("build_only"));
	}
	string tag_tmp;
	for (unsigned int i=0; i<pkg.get_tags()->size(); i++)
	{
		tag_tmp=pkg.get_tags()->at(i);
		ui.TagListWidget->addItem(tag_tmp.c_str());
		tag_tmp.clear();
	}
	for (unsigned int i=0; i<pkg.get_config_files()->size(); i++)
	{
		QListWidgetItem *__item = new QListWidgetItem(ui.configFilesListWidget);
		__item->setText(pkg.get_config_files()->at(i).get_name()->c_str());
	}
	for (unsigned int i=0; i<pkg.get_temp_files()->size(); i++)
	{
		QListWidgetItem *__item = new QListWidgetItem(ui.tempFilesListWidget);
		__item->setText(pkg.get_temp_files()->at(i).get_name()->c_str());
	}
	reloadPackageDirListing();
	reloadFilesystemDirListing();
}

bool Form::saveFile()
{
	// Check if all required fields are filled in
	if (ui.NameEdit->text().isEmpty() || ui.VersionEdit->text().isEmpty() || ui.BuildEdit->text().isEmpty()) {
		QMessageBox::warning(this, "Some required fields are empty", "Please fill in all required fields (name, arch, version, build) first.");
		return false;
	}
	QString out_dir;
	if (dataType==DATATYPE_NEW)
	{
		out_dir = QFileDialog::getExistingDirectory(this, tr("Where to save the package:"), "./");
		if (out_dir.isEmpty()) return false;

		if (ui.sourcePackageRadioButton->isChecked()) {
			dataType = DATATYPE_SOURCEPACKAGE;
			sourcePackage.createNew();
		}

		if (ui.binaryPackageRadioButton->isChecked()) {
			dataType = DATATYPE_BINARYPACKAGE;
			binaryPackage.createNew();
		}
		if (dataType == DATATYPE_NEW)
		{
			QMessageBox::warning(this, "No package type specified", "Please specify package data first");
			return false;
		}
	}


	string xmlDir, xmlFilename;
	switch(dataType)
	{
		case DATATYPE_BINARYPACKAGE:
			xmlFilename = binaryPackage.getDataFilename();
			xmlDir = getDirectory(binaryPackage.getDataFilename());
			break;
		case DATATYPE_SOURCEPACKAGE:
			xmlFilename = sourcePackage.getDataFilename();
			xmlDir = getDirectory(sourcePackage.getDataFilename());
			break;
		default:
			mError("This type of file isn't supported");
			return false;
	}
	QString currentWindowTitle = windowTitle();
	setWindowTitle(windowTitle()+tr(": saving, please wait..."));
	modified=false;

	string slack_desc;
	string slack_required;
	string slack_suggests;
	string desc_chunk;


	XMLNode node;
	node = XMLNode::createXMLTopNode("package");
	node.addChild("name");
	node.getChildNode("name").addText(ui.NameEdit->text().toStdString().c_str());
	node.addChild("version");
	node.getChildNode("version").addText(ui.VersionEdit->text().toStdString().c_str());
	if (!ui.betaReleaseEdit->text().isEmpty()) {
		node.addChild("betarelease");
		node.getChildNode("betarelease").addText(ui.betaReleaseEdit->text().toStdString().c_str());
	}
	node.addChild("arch");
	node.getChildNode("arch").addText(ui.ArchComboBox->currentText().toStdString().c_str());
	node.addChild("build");
	node.getChildNode("build").addText(ui.BuildEdit->text().toStdString().c_str());

	storeCurrentDescription();

	node.addChild("description");
	node.getChildNode("description",0).addAttribute("lang", "en");
	node.getChildNode("description",0).addText(description[0].toStdString().c_str());
	unsigned int spacePosition=0;
	slack_desc = ui.NameEdit->text().toStdString() + ": " + short_description[0].toStdString() + "\n" + ui.NameEdit->text().toStdString() + ": \n";
	for (unsigned int i=0; i<description[0].toStdString().length(); i++)
	{
		if (description[0].toStdString()[i]==' ') spacePosition = i;
		if (i >=70 || i == description[0].toStdString().length()-1)
		{
			desc_chunk = ui.NameEdit->text().toStdString() + ": " + description[0].toStdString().substr(0, spacePosition);
			if (i < description[0].toStdString().length()-1) i=0;
			if (spacePosition < description[0].toStdString().length()) description[0] = description[0].toStdString().substr(spacePosition).c_str();
			slack_desc+=desc_chunk + "\n";
		}
	}
	slack_desc = slack_desc.substr(0, slack_desc.length()-1) + description[0].toStdString();

	node.addChild("description");
	node.getChildNode("description",1).addAttribute("lang", "ru");
	node.getChildNode("description",1).addText(description[1].toStdString().c_str());

	node.addChild("short_description");
	node.getChildNode("short_description",0).addAttribute("lang", "en");
	node.getChildNode("short_description",0).addText(short_description[0].toStdString().c_str());
	node.addChild("short_description");
	node.getChildNode("short_description",1).addAttribute("lang", "ru");
	node.getChildNode("short_description",1).addText(short_description[1].toStdString().c_str());

	node.addChild("dependencies");

	node.addChild("suggests");
	int dcurr=0;
	int scurr=0;
	printf("Deps...\n");
	for (int i=0; i<ui.DepTableWidget->rowCount(); i++)
	{
		printf("[%d] Starting\n", i);
		if (ui.DepTableWidget->item(i,3)->text().toUpper()== "DEPENDENCY")
		{
			node.getChildNode("dependencies").addChild("dep");
			node.getChildNode("dependencies").getChildNode("dep", dcurr).addChild("name");
			node.getChildNode("dependencies").getChildNode("dep", dcurr).getChildNode("name").addText(ui.DepTableWidget->item(i,0)->text().toStdString().c_str());
			slack_required+=ui.DepTableWidget->item(i,0)->text().toStdString();
			node.getChildNode("dependencies").getChildNode("dep", dcurr).addChild("condition");
			node.getChildNode("dependencies").getChildNode("dep", dcurr).getChildNode("condition").addText(hcondition2xml(ui.DepTableWidget->item(i,1)->text().toStdString()).c_str());
			if (ui.DepTableWidget->item(i,1)->text().toStdString()!="any")
			{
				if (ui.DepTableWidget->item(i,1)->text().toStdString() == "==")
				{
					slack_required += "=";
				}
				else
				{
					slack_required+=ui.DepTableWidget->item(i,1)->text().toStdString();
				}
			}
			node.getChildNode("dependencies").getChildNode("dep", dcurr).addChild("version");
			node.getChildNode("dependencies").getChildNode("dep", dcurr).getChildNode("version").addText(ui.DepTableWidget->item(i,2)->text().toStdString().c_str());
			printf("[%d] Name, condition, version done\n", i);
			printf("SIZES: %dx%d\n", ui.DepTableWidget->rowCount(), ui.DepTableWidget->columnCount());
			if (false) ///!ui.DepTableWidget->item(i,4)->text().isEmpty()/* == "build_only"*/)
			{
				node.getChildNode("dependencies").getChildNode("dep",dcurr).addChild("build_only");
				node.getChildNode("dependencies").getChildNode("dep",dcurr).getChildNode("build_only").addText("true");
			}
			printf("[%d] build_only flag set\n",i);
			if (ui.DepTableWidget->item(i,1)->text().toStdString()!="any")
			{
				slack_required += ui.DepTableWidget->item(i,2)->text().toStdString() + "\n";
			}
			dcurr++;
		}
		if (ui.DepTableWidget->item(i,3)->text().toUpper()=="SUGGESTION" ||
				ui.DepTableWidget->item(i,3)->text().toUpper()=="SUGGEST")
		{
			node.getChildNode("suggests").addChild("suggest");
			node.getChildNode("suggests").getChildNode("suggest", scurr).addChild("name");
			node.getChildNode("suggests").getChildNode("suggest", scurr).getChildNode("name").addText(ui.DepTableWidget->item(i,0)->text().toStdString().c_str());
			node.getChildNode("suggests").getChildNode("suggest", scurr).addChild("condition");
			node.getChildNode("suggests").getChildNode("suggest", scurr).getChildNode("condition").addText(hcondition2xml(ui.DepTableWidget->item(i,1)->text().toStdString()).c_str());
			node.getChildNode("suggests").getChildNode("suggest", scurr).addChild("version");
			node.getChildNode("suggests").getChildNode("suggest", scurr).getChildNode("version").addText(ui.DepTableWidget->item(i,2)->text().toStdString().c_str());
			scurr++;
		}

	}
	node.addChild("tags");
	node.addChild("changelog");

	for (int i=0; i<ui.TagListWidget->count(); i++)
	{
		node.getChildNode("tags").addChild("tag");
		node.getChildNode("tags").getChildNode("tag",i).addText(ui.TagListWidget->item(i)->text().toStdString().c_str());
	}

	node.getChildNode("changelog").addText(ui.ChangelogEdit->toPlainText().toStdString().c_str());
	if (!ui.MaintainerNameEdit->text().isEmpty())
	{
		node.addChild("maintainer");
		node.getChildNode("maintainer").addChild("name");
		node.getChildNode("maintainer").getChildNode("name").addText(ui.MaintainerNameEdit->text().toStdString().c_str());
		if (!ui.MaintainerMailEdit->text().isEmpty())
		{
			node.getChildNode("maintainer").addChild("email");
			node.getChildNode("maintainer").getChildNode("email").addText(ui.MaintainerMailEdit->text().toStdString().c_str());
			if (mConfig.getValue("maintainer_email").empty()) mConfig.setValue("maintainer_email", ui.MaintainerMailEdit->text().toStdString());
		}
		if (mConfig.getValue("maintainer_name").empty()) mConfig.setValue("maintainer_name", ui.MaintainerNameEdit->text().toStdString());
	}
	node.addChild("configfiles");
	for (int i=0; i<ui.configFilesListWidget->count(); i++)
	{
		node.getChildNode("configfiles").addChild("conffile");
		node.getChildNode("configfiles").getChildNode("conffile",i).addText(ui.configFilesListWidget->item(i)->text().toStdString().c_str());
	}

	node.addChild("tempfiles");
	for (int i=0; i<ui.tempFilesListWidget->count(); i++)
	{
		node.getChildNode("tempfiles").addChild("tempfile");
		node.getChildNode("tempfiles").getChildNode("tempfile").addText(ui.tempFilesListWidget->item(i)->text().toStdString().c_str());
	}

	// Mbuild-related
	if (!ui.urlEdit->text().isEmpty())
	{
		node.addChild("mbuild");
		node.getChildNode("mbuild").addChild("url");
		node.getChildNode("mbuild").getChildNode("url").addText(ui.urlEdit->text().toStdString().c_str());
		node.getChildNode("mbuild").addChild("patches");
		for (unsigned int i=0; i<patchList.size(); i++)
		{
			node.getChildNode("mbuild").getChildNode("patches").addChild("patch");
			node.getChildNode("mbuild").getChildNode("patches").getChildNode("patch",i).addText(getFilename(patchList[i]).c_str());
		}
		node.getChildNode("mbuild").addChild("sources_root_directory");
		if (ui.sourcesRootDirectoryAutodetectCheckBox->checkState()==Qt::Unchecked) 
			node.getChildNode("mbuild").getChildNode("sources_root_directory").addText(ui.sourcesRootDirectoryEdit->text().toStdString().c_str());
		node.getChildNode("mbuild").addChild("build_system");
		switch(ui.buildingSystemComboBox->currentIndex())
		{
			case 0: node.getChildNode("mbuild").getChildNode("build_system").addText("autotools");
				break;
			case 1: node.getChildNode("mbuild").getChildNode("build_system").addText("scons");
				break;
			case 2: node.getChildNode("mbuild").getChildNode("build_system").addText("cmake");
				break;
			case 3: node.getChildNode("mbuild").getChildNode("build_system").addText("custom");
				break;
			case 4: node.getChildNode("mbuild").getChildNode("build_system").addText("script");
				break;
		}

		node.getChildNode("mbuild").addChild("max_numjobs");
		node.getChildNode("mbuild").getChildNode("max_numjobs").addText(ui.maxNumjobsSpinBox->cleanText().toStdString().c_str());
		
		node.getChildNode("mbuild").addChild("optimization");
		if (ui.cpuArchCheckBox->checkState()==Qt::Checked) {
			node.getChildNode("mbuild").getChildNode("optimization").addChild("march");
			node.getChildNode("mbuild").getChildNode("optimization").getChildNode("march").addText(ui.cpuArchComboBox->currentText().toStdString().c_str());
		}
		if (ui.cpuTuneCheckBox->checkState()==Qt::Checked) {
			node.getChildNode("mbuild").getChildNode("optimization").addChild("mtune");
			node.getChildNode("mbuild").getChildNode("optimization").getChildNode("mtune").addText(ui.cpuTuneComboBox->currentText().toStdString().c_str());
		}
		if (ui.optimizationCheckBox->checkState()==Qt::Checked) {
			node.getChildNode("mbuild").getChildNode("optimization").addChild("olevel");
			node.getChildNode("mbuild").getChildNode("optimization").getChildNode("olevel").addText(ui.optimizationComboBox->currentText().toStdString().c_str());
		}
		if (ui.customGccOptionsCheckBox->checkState()==Qt::Checked) {
			node.getChildNode("mbuild").getChildNode("optimization").addChild("custom_gcc_options");
			node.getChildNode("mbuild").getChildNode("optimization").getChildNode("custom_gcc_options").addText(ui.customGccOptionsEdit->text().toStdString().c_str());
		}

		node.getChildNode("mbuild").getChildNode("optimization").addChild("allow_change");
		if (ui.AllowUserToChangeCheckBox->checkState()==Qt::Checked)
			node.getChildNode("mbuild").getChildNode("optimization").getChildNode("allow_change").addText("true");
		else   	
			node.getChildNode("mbuild").getChildNode("optimization").getChildNode("allow_change").addText("false");
	
		node.getChildNode("mbuild").addChild("use_cflags");	
		if (ui.dontUseCflags->isChecked())
			node.getChildNode("mbuild").getChildNode("use_cflags").addText("false");
		else
			node.getChildNode("mbuild").getChildNode("use_cflags").addText("true");

		if (ui.envOptionsCheckBox->checkState()==Qt::Checked)
		{
			node.getChildNode("mbuild").addChild("env_options");
			node.getChildNode("mbuild").getChildNode("env_options").addText(ui.envOptionsEdit->text().toStdString().c_str());
		}
		node.getChildNode("mbuild").addChild("configuration");
		for (unsigned int i=0; i<keyList.size(); i++)
		{
			node.getChildNode("mbuild").getChildNode("configuration").addChild("key");
			node.getChildNode("mbuild").getChildNode("configuration").getChildNode("key",i).addChild("name");
			node.getChildNode("mbuild").getChildNode("configuration").getChildNode("key",i).getChildNode("name").addText(keyList[i].name.c_str());
			node.getChildNode("mbuild").getChildNode("configuration").getChildNode("key",i).addChild("value");
			node.getChildNode("mbuild").getChildNode("configuration").getChildNode("key",i).getChildNode("value").addText(keyList[i].value.c_str());
		}
		if (ui.buildingSystemComboBox->currentIndex()==3)
		{
			node.getChildNode("mbuild").addChild("custom_commands");
			if (ui.configureCheckBox->checkState()==Qt::Checked)
			{
				node.getChildNode("mbuild").getChildNode("custom_commands").addChild("configure");
				node.getChildNode("mbuild").getChildNode("custom_commands").getChildNode("configure").addText(ui.configureEdit->text().toStdString().c_str());
			}
			if (ui.compilationCheckBox->checkState()==Qt::Checked)
			{
				node.getChildNode("mbuild").getChildNode("custom_commands").addChild("make");
				node.getChildNode("mbuild").getChildNode("custom_commands").getChildNode("make").addText(ui.compilationEdit->text().toStdString().c_str());
			}
			if (ui.installationCheckBox->checkState()==Qt::Checked)
			{
				node.getChildNode("mbuild").getChildNode("custom_commands").addChild("make_install");
				node.getChildNode("mbuild").getChildNode("custom_commands").getChildNode("make_install").addText(ui.installEdit->text().toStdString().c_str());
			}
		}
	}

	node.writeToFile(xmlFilename.c_str());

	setWindowTitle(currentWindowTitle+tr(" (saved)"));
	modified=false;
	switch(dataType)
	{
		case DATATYPE_BINARYPACKAGE:
			binaryPackage.packFile(out_dir.toStdString());
			break;
		case DATATYPE_SOURCEPACKAGE:
			if (ui.embedSourcesCheckBox->isChecked()) embedSources();
			else sourcePackage.removeSource();
			if (patchList.empty()) sourcePackage.removeAllPatches();
			embedPatches();
			sourcePackage.setBuildScript(ui.customScriptTextEdit->toPlainText().toStdString());
			sourcePackage.packFile(out_dir.toStdString());
			break;
	}

	if (WriteFile(xmlDir+"/slack-desc", slack_desc)!=0) {
		QMessageBox::critical(this, "Error saving package", "Error while saving package");
		return false;
	}

	if (!slack_required.empty()) WriteFile(xmlDir+"/slack-required", slack_required);
	reloadPackageDirListing();
	reloadFilesystemDirListing();
	return true;
}
void Form::showAbout()
{
	QMessageBox::information(this, tr("About packagebuilder"), tr("Package metadata builder for MPKG ") + (QString) mpkgVersion + " (build " + (QString) mpkgBuild + tr(")\n\n(c) RPU NET (www.rpunet.ru)\nLicensed under GPL"), QMessageBox::Ok, QMessageBox::Ok);
}
void Form::loadBuildScriptFromFile()
{
	QString importFileName = QFileDialog::getOpenFileName(this, tr("Select a script file"));
	if (importFileName.isEmpty()) return;
	if (!FileExists(importFileName.toStdString())) {
		QMessageBox::warning(this, "Error importing script", "The specified file doesn't exist");
		return;
	}
	ui.customScriptTextEdit->setPlainText(ReadFile(importFileName.toStdString()).c_str());
}
	

void Form::editBuildScriptWithGvim()
{
	if (tempScriptEditFile.empty())
	{
		tempScriptEditFile = get_tmp_file();
		WriteFile(tempScriptEditFile, ui.customScriptTextEdit->toPlainText().toStdString());
		system("gvim " + tempScriptEditFile);
		ui.editWithGvimButton->setText(tr("Finish editing with gvim"));
	}
	else {
		ui.customScriptTextEdit->setPlainText(ReadFile(tempScriptEditFile).c_str());
		unlink(tempScriptEditFile.c_str());
		tempScriptEditFile.clear();
		ui.editWithGvimButton->setText(tr("Edit with gvim"));
	}
}

void Form::addTag(){
	if (!ui.tagBox->currentText().isEmpty())
	{
		ui.TagListWidget->addItem(ui.tagBox->currentText());
	}
}

void Form::saveAndExit()
{
	if (saveFile()) quitApp();
}
void Form::addDepsFromFiles()
{
	QStringList files;
	QFileDialog *dialog = new QFileDialog;
	dialog->setFileMode(QFileDialog::ExistingFiles);
	if (dialog->exec()) files = dialog->selectedFiles();
	PackageConfig *p;
	string tmp_xml = get_tmp_file();
	int e_res=-1;
	for (int i=0; i<files.size(); i++)
	{
		if (getExtension(files.at(i).toStdString())=="tgz")
			e_res = extractFromTgz(files.at(i).toStdString(), "install/data.xml", tmp_xml);
		if (getExtension(files.at(i).toStdString())=="spkg")
			e_res = extractFromTar(files.at(i).toStdString(), "install/data.xml", tmp_xml);
		if (e_res==0)
		{
			p = new PackageConfig(tmp_xml);
			ui.DepNameEdit->setText(p->getName().c_str());
			ui.DepConditionComboBox->setCurrentIndex(1);
			ui.DepVersionEdit->setText(p->getVersion().c_str());
			addDependency();
			delete p;
			unlink(tmp_xml.c_str());
		}
		else QMessageBox::warning(this, tr("Error importing dependencies"), tr("Cannot import dependency from file ") + files.at(i), QMessageBox::Ok, QMessageBox::Ok);
	}
}

void Form::addDependency(){

	if (!ui.DepNameEdit->text().isEmpty())
	{
		if (ui.DepConditionComboBox->currentText()=="any" || !ui.DepVersionEdit->text().isEmpty())
		{
			ui.DepTableWidget->insertRow(0);
			ui.DepTableWidget->setItem(0,3, new QTableWidgetItem(ui.DepSuggestComboBox->currentText()));
			ui.DepTableWidget->setItem(0,0, new QTableWidgetItem(ui.DepNameEdit->text()));
			ui.DepTableWidget->setItem(0,1, new QTableWidgetItem(ui.DepConditionComboBox->currentText()));
			ui.DepTableWidget->setItem(0,2, new QTableWidgetItem(ui.DepVersionEdit->text()));
			if (ui.buildDependencyCheckBox->isChecked()) ui.DepTableWidget->setItem(0,4, new QTableWidgetItem("build_only"));
			else ui.DepTableWidget->setItem(0,4,new QTableWidgetItem(""));
			ui.DepNameEdit->clear();
			ui.DepVersionEdit->clear();
			ui.DepSuggestComboBox->setCurrentIndex(0);
			ui.DepConditionComboBox->setCurrentIndex(0);
		}
	}
}
void Form::addConfigFile()
{
	if (!ui.confFileAddEdit->text().isEmpty())
	{
		QListWidgetItem *__item = new QListWidgetItem(ui.configFilesListWidget);
		__item->setText(ui.confFileAddEdit->text());
	}
}

void Form::deleteConfigFile()
{
	int i=ui.configFilesListWidget->currentRow();
	ui.configFilesListWidget->takeItem(i);
}
void Form::deleteTag()
{
	int i=ui.TagListWidget->currentRow();
	ui.TagListWidget->takeItem(i);
}
void Form::searchConfigFile()
{
	QString pkgRoot;
	if (dataType==DATATYPE_BINARYPACKAGE) pkgRoot = binaryPackage.getExtractedPath().c_str();

	if (pkgRoot.isEmpty())
	{
		pkgRoot = QFileDialog::getExistingDirectory(this, tr("Choose package root"), "./");
	}
	if (!pkgRoot.isEmpty()) 
	{
		QString fname = QFileDialog::getOpenFileName(this, tr("Choose a config file"), pkgRoot, "");
		if (fname.length()>pkgRoot.length())
		ui.confFileAddEdit->setText(fname.toStdString().substr(pkgRoot.length()).c_str());
	}
}
void Form::searchTempFile()
{
	QString pkgRoot;
	if (dataType==DATATYPE_BINARYPACKAGE) pkgRoot = binaryPackage.getExtractedPath().c_str();

	if (pkgRoot.isEmpty())
	{
		pkgRoot = QFileDialog::getExistingDirectory(this, tr("Choose package root"), "./");
	}
	if (!pkgRoot.isEmpty()) 
	{
		QString fname = QFileDialog::getOpenFileName(this, tr("Choose a config file"), pkgRoot, "");
		if (fname.length()>pkgRoot.length())
		ui.tempFileAddEdit->setText(fname.toStdString().substr(pkgRoot.length()).c_str());
	}
}

void Form::deleteDependency()
{
	int i=ui.DepTableWidget->currentRow();
	ui.DepTableWidget->removeRow(i);
}
void Form::changeHeader()
{
	modified=true;

	QString FLabel=tr("MOPSLinux package builder");

	if (!ui.NameEdit->text().isEmpty())
	{
		FLabel+=": "+ui.NameEdit->text();
		if (!ui.VersionEdit->text().isEmpty())
		{
			FLabel+="-"+ui.VersionEdit->text()+"-"+ui.ArchComboBox->currentText();
			if (!ui.BuildEdit->text().isEmpty())
			{
				FLabel+="-"+ui.BuildEdit->text();
			}
		}
	}
	setWindowTitle(FLabel);
}

void Form::changeHeader(const QString &)
{
	/*if (!text.isEmpty())
	{
		setWindowTitle(text);
		return;
	}*/
	modified=true;
	QString FLabel=tr("MOPSLinux package builder");

	if (!ui.NameEdit->text().isEmpty())
	{
		FLabel+=": "+ui.NameEdit->text();
		if (!ui.VersionEdit->text().isEmpty())
		{
			FLabel+="-"+ui.VersionEdit->text()+"-"+ui.ArchComboBox->currentText();
			if (!ui.BuildEdit->text().isEmpty())
			{
				FLabel+="-"+ui.BuildEdit->text();
			}
		}
	}
	setWindowTitle(FLabel);
}

void Form::swapLanguage()
{
	
	int i;
	int i2;
	/*if (ui.DescriptionLanguageComboBox->currentText()=="ru")
	{
		i=0;
		i2=1;
	}
	else 
	{*/
		i=1;
		i2=0;
	//}

	short_description[i]=ui.ShortDescriptionEdit->text();
	description[i]=ui.DescriptionEdit->toPlainText();
	ui.ShortDescriptionEdit->setText(short_description[i2]);
	ui.DescriptionEdit->setPlainText(description[i2]);
}

void Form::storeCurrentDescription()
{
	int i;
	/*if (ui.DescriptionLanguageComboBox->currentText()=="ru")
	{
		i=1;
	}
	else 
	{*/
		i=0;
	//}

	short_description[i]=ui.ShortDescriptionEdit->text();
	description[i]=ui.DescriptionEdit->toPlainText();

}

void Form::quitApp()
{
	int ret;
	if (modified)
	{
		ret = QMessageBox::warning(this, tr("MOPSLinux package builder"),
                   tr("The document has been modified.\n"
                      "Do you want to save your changes?"),
                   QMessageBox::Save | QMessageBox::Discard
                   | QMessageBox::Cancel,
                   QMessageBox::Save);
		switch(ret)
		{
			case QMessageBox::Save: 
				saveFile();
				qApp->quit();
				break;
			case QMessageBox::Discard:
				qApp->quit();
				break;
			case QMessageBox::Cancel:
				break;
		}
	}
	else qApp->quit();
}

void Form::switchEnvField(int state)
{
	if (state==Qt::Checked) ui.envOptionsEdit->setEnabled(true);
	else ui.envOptionsEdit->setEnabled(false);
}
void Form::switchConfigureField(int state)
{
	if (state==Qt::Checked) ui.configureEdit->setEnabled(true);
	else ui.configureEdit->setEnabled(false);
}
void Form::switchCompilationField(int state)
{
	if (state==Qt::Checked) ui.compilationEdit->setEnabled(true);
	else ui.compilationEdit->setEnabled(false);

}
void Form::switchInstallField(int state)
{
	if (state==Qt::Checked) ui.installEdit->setEnabled(true);
	else ui.installEdit->setEnabled(false);

}
void Form::switchSourceDirectoryField(int state)
{
	if (state==Qt::Checked) ui.sourcesRootDirectoryEdit->setEnabled(false);
	else ui.sourcesRootDirectoryEdit->setEnabled(true);

}
void Form::switchBuildSystem(int index)
{
	if (index<3) {
		ui.optimizationGroupBox->setVisible(true);
		ui.customScriptGroupBox->setVisible(false);
		ui.customCommandsGroupBox->setVisible(false);
		ui.compilationGroupBox->setVisible(true);

	}
	if (index==4)
	{
		ui.optimizationGroupBox->setVisible(false);
		ui.customScriptGroupBox->setVisible(true);
		ui.customCommandsGroupBox->setVisible(false);
		ui.compilationGroupBox->setVisible(false);

	}
	if (index==3)
	{
		ui.optimizationGroupBox->setVisible(true);
		ui.customScriptGroupBox->setVisible(false);
		ui.customCommandsGroupBox->setVisible(true);
		ui.compilationGroupBox->setVisible(true);
	}


}
void Form::switchCpuArchField(int state)
{
	if (state==Qt::Checked) ui.cpuArchComboBox->setEnabled(true);
	else ui.cpuArchComboBox->setEnabled(false);

}
void Form::switchCpuTuneField(int state)
{
	if (state==Qt::Checked) ui.cpuTuneComboBox->setEnabled(true);
	else ui.cpuTuneComboBox->setEnabled(false);

}
void Form::switchOptimizationField(int state)
{
	if (state==Qt::Checked) ui.optimizationComboBox->setEnabled(true);
	else ui.optimizationComboBox->setEnabled(false);

}
void Form::switchGccOptionsField(int state)
{
	if (state==Qt::Checked) ui.customGccOptionsEdit->setEnabled(true);
	else ui.customGccOptionsEdit->setEnabled(false);

}

void Form::displayKeys()
{
	ui.compilationOptionsTableWidget->clear();
	ui.compilationOptionsTableWidget->setColumnCount(2);
	ui.compilationOptionsTableWidget->setRowCount(keyList.size());
	for (unsigned int i=0; i<keyList.size(); i++)
	{
		ui.compilationOptionsTableWidget->setItem(i,0, new QTableWidgetItem(keyList[i].name.c_str()));
		ui.compilationOptionsTableWidget->setItem(i,1, new QTableWidgetItem(keyList[i].value.c_str()));
	}

}

void Form::displayPatches()
{
	ui.patchListWidget->clear();
	for (unsigned int i=0; i<patchList.size(); i++)
	{
		QListWidgetItem *__item = new QListWidgetItem(ui.patchListWidget);
		__item->setText(patchList[i].c_str());
	}
}
void Form::browsePatch()
{
	QString patchName = QFileDialog::getOpenFileName(this, tr("Open a package (.tgz or .spkg)"), "");
	if (patchName.isEmpty()) return;
	ui.patchEdit->setText(patchName);
}

void Form::addPatch()
{
	if (!ui.patchEdit->text().isEmpty()) {
		patchList.push_back(ui.patchEdit->text().toStdString());
		ui.patchEdit->setText("");
		displayPatches();
	}
}

void Form::addKey()
{
	if (!ui.keyNameEdit->text().isEmpty())
	{
		keys key_tmp;
		key_tmp.name = ui.keyNameEdit->text().toStdString();
		key_tmp.value = ui.keyValueEdit->text().toStdString();
		keyList.push_back(key_tmp);
		displayKeys();
		ui.keyNameEdit->clear();
		ui.keyValueEdit->clear();
	}
}

void Form::deletePatch()
{
	int i = ui.patchListWidget->currentRow();
	if (i>=0 && (unsigned int) i<patchList.size())
	{
		vector<string> copy;
		for (unsigned int t=0; t<patchList.size(); t++)
		{
			if (t!=(unsigned int) i) copy.push_back(patchList[t]);
		}
		patchList=copy;
		displayPatches();
	}
}

void Form::deleteKey()
{
	int i = ui.compilationOptionsTableWidget->currentRow();
	if (i>=0 && (unsigned int) i<keyList.size())
	{
		vector<keys> copy;
		for (unsigned int t=0; t<keyList.size(); t++)
		{
			if (t!=(unsigned int) i) copy.push_back(keyList[t]);
		}
		keyList=copy;
		displayKeys();
	}
}
void Form::setNewPackageCurrentDirectory(QListWidgetItem *item)
{

	prevPackageDirName = currentPackageDir->dirName();
	QFileInfoList list = currentPackageDir->entryInfoList();
	currentPackageDir->cd(list.at(ui.filelistWidget->row(item)+1).fileName());
	reloadPackageDirListing();
	QList<QListWidgetItem *> items;
	items = ui.filelistWidget->findItems(prevPackageDirName, Qt::MatchFixedString | Qt::MatchCaseSensitive);
	if (items.size()>0) ui.filelistWidget->setCurrentItem(items[0]);
}
void Form::reloadPackageDirListing() // Fills the list
{
	setCurrentPathLabel();
	if (!currentPackageDir->exists() || dataType == DATATYPE_NEW) {
		return;
	}
	ui.filelistWidget->clear();
	ui.filelistCurrentPath->setText(currentPackageDir->canonicalPath());
	QFileInfoList list = currentPackageDir->entryInfoList();
	if (list.size()==0) return;
	for (int i=1; i<list.size(); i++) {
		QListWidgetItem *__item = new QListWidgetItem(ui.filelistWidget);
		__item->setText(list.at(i).fileName());
		if (list.at(i).isDir())	__item->setIcon(QIcon("/usr/share/mpkg/packagebuilder/icons/folder.png"));
		else __item->setIcon(QIcon("/usr/share/mpkg/packagebuilder/icons/source.png"));
	}
	if (list.size()>0) ui.filelistWidget->setCurrentRow(0);
}

void Form::setNewFilesystemCurrentDirectory(QListWidgetItem *item)
{

	prevFilesystemDirName = currentFilesystemDir->dirName();
	QFileInfoList list = currentFilesystemDir->entryInfoList();
	currentFilesystemDir->cd(list.at(ui.filelistWidget_2->row(item)+1).fileName());
	reloadFilesystemDirListing();
	QList<QListWidgetItem *> items;
	items = ui.filelistWidget_2->findItems(prevFilesystemDirName, Qt::MatchFixedString | Qt::MatchCaseSensitive);
	if (items.size()>0) ui.filelistWidget_2->setCurrentItem(items[0]);

}
void Form::reloadFilesystemDirListing() // Fills the list
{
	setCurrentPathLabel();

	if (!currentFilesystemDir->exists()) return;
	ui.filelistWidget_2->clear();
	ui.filelistCurrentPath_2->setText(currentFilesystemDir->canonicalPath());
	QFileInfoList list = currentFilesystemDir->entryInfoList();
	if (list.size()==0) return;
	for (int i=1; i<list.size(); i++) {
		QListWidgetItem *__item = new QListWidgetItem(ui.filelistWidget_2);
		__item->setText(list.at(i).fileName());
		if (list.at(i).isDir())	__item->setIcon(QIcon("/usr/share/mpkg/packagebuilder/icons/folder.png"));
		else __item->setIcon(QIcon("/usr/share/mpkg/packagebuilder/icons/source.png"));
	}
	if (list.size()>1) ui.filelistWidget_2->setCurrentRow(0);
}

void Form::goPackageHome()
{
	*currentPackageDir = *packageDir;
	reloadPackageDirListing();
}

void Form::goPackageUp()
{
	currentPackageDir->cdUp();
	reloadPackageDirListing();
}

void Form::goFilesystemHome()
{
	*currentFilesystemDir = QDir::homePath();
	reloadFilesystemDirListing();//DirListing();
}

void Form::goFilesystemUp()
{
	currentFilesystemDir->cdUp();
	reloadFilesystemDirListing();
}
void Form::copyFiles()
{
	manageFiles(FACT_COPY);
}
void Form::manageFiles(FileAction action)
{
	//TODO: fix focus detection
	int direction=focusIndex;
	printf("Direction: %d\n", direction);
	QFileInfo list;
	QFile operatedFile;
	int result=0;
	QString source, dest;
	QList<QListWidgetItem *> itemList;
	QListWidget *widget;
	QDir *curr, *opposite;
	switch(direction)
	{
		case 1: widget = ui.filelistWidget;
			curr = currentPackageDir;
			opposite = currentFilesystemDir;
			break;
		case 2:
			widget = ui.filelistWidget_2;
			curr = currentFilesystemDir;
			opposite = currentPackageDir;
			break;
		default: return;
	}
	itemList = widget->selectedItems();
	printf("itemList size = %d\n", itemList.size());
	for (int i=0; i<itemList.size(); i++) {
		printf("Processing element %d\n",i);
		list = curr->entryInfoList().at(widget->row(itemList[i])+1);
		if (list.fileName()==".." || list.fileName()==".") continue;
		source = list.absoluteFilePath();
		dest = opposite->absolutePath()+"/"+list.fileName();
		printf("Source: [%s]\nDestination: [%s]\n\n", source.toStdString().c_str(), dest.toStdString().c_str());
		if (action==FACT_COPY) {
			//dest = QInputDialog::getText(this, tr("Copy file"), tr("Copy \"") + source + tr("\" to:"), QLineEdit::Normal, dest);
			if (dest.isEmpty()) return;
			result = copyFile(source.toStdString(), dest.toStdString());
		}
		if (action==FACT_MOVE) {
			//dest = QInputDialog::getText(this, tr("Move file"), tr("Move \"") + source + tr("\" to:"), QLineEdit::Normal, dest);
			if (dest.isEmpty()) return;

			result = moveFile(source.toStdString(), dest.toStdString());
		}
		if (action==FACT_REMOVE) {
			if (QMessageBox::question(this, tr("Remove file"), tr("Remove \"") + source + tr("\" ?"), QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok)!=QMessageBox::Ok) return;
			result = removeFile(source.toStdString());
		}
		printf("Result: %d\n", result);
	}
	reloadFilesystemDirListing();
	reloadPackageDirListing();
}
void Form::renameFile()
{
	QString newName;
	QFileInfo fileInfo;
	switch(focusIndex)
	{
		case 1:
			fileInfo = currentPackageDir->entryInfoList().at(ui.filelistWidget->currentRow()+1);
			if (fileInfo.fileName()==".." || fileInfo.fileName()==".") return;
			newName = QInputDialog::getText(this, tr("Rename file"), tr("Enter a new file name"), QLineEdit::Normal, fileInfo.fileName());
			if (!newName.isEmpty()) QFile::rename(fileInfo.absoluteFilePath(), currentPackageDir->absolutePath()+"/"+newName);
			break;
		case 2:
			fileInfo = currentFilesystemDir->entryInfoList().at(ui.filelistWidget_2->currentRow()+1);
			if (fileInfo.fileName()==".." || fileInfo.fileName()==".") return;
			newName = QInputDialog::getText(this, tr("Rename file"), tr("Enter a new file name"), QLineEdit::Normal, fileInfo.fileName());
			if (!newName.isEmpty()) QFile::rename(fileInfo.absoluteFilePath(), currentFilesystemDir->absolutePath()+"/"+newName);
			break;
	}
	reloadPackageDirListing();
	reloadFilesystemDirListing();
}
void Form::moveFiles()
{
	manageFiles(FACT_MOVE);
}

void Form::removeFiles()
{
	manageFiles(FACT_REMOVE);
}

void Form::viewFile()
{
	QTextBrowser *browser = new QTextBrowser;
	QFileInfo fileInfo;
	switch(focusIndex)
	{
		case 1:
			fileInfo = currentPackageDir->entryInfoList().at(ui.filelistWidget->currentRow()+1);
			break;
		case 2:
			fileInfo = currentFilesystemDir->entryInfoList().at(ui.filelistWidget_2->currentRow()+1);
			break;
	}
	browser->setText(ReadFile(fileInfo.absoluteFilePath().toStdString()).c_str());
	browser->showMaximized();

}

void Form::editFile()
{
	QFileInfo fileInfo;
	switch(focusIndex)
	{
		case 1:
			fileInfo = currentPackageDir->entryInfoList().at(ui.filelistWidget->currentRow()+1);
			break;
		case 2:
			fileInfo = currentFilesystemDir->entryInfoList().at(ui.filelistWidget_2->currentRow()+1);
			break;
	}
	system("gvim " + fileInfo.absoluteFilePath().toStdString());
}

void Form::makeDirectory()
{
	QString dirName = QInputDialog::getText(this, tr("Make new directory"), tr("Enter new directory name:"));
	if (dirName.isEmpty()) return;
	QDir *curr;
	QListWidget *widget;
	switch(focusIndex) {
		case 1:
			curr = currentPackageDir;
			widget = ui.filelistWidget;
			break;
		case 2:
			curr = currentFilesystemDir;
			widget = ui.filelistWidget_2;
			break;
		default: return;
	}
	curr->mkdir(dirName);
	reloadFilesystemDirListing();
	reloadPackageDirListing();
	QList <QListWidgetItem *> items = widget->findItems(dirName, Qt::MatchFixedString | Qt::MatchCaseSensitive);
	if (items.size()>0) widget->setCurrentItem(items[0]);
}
