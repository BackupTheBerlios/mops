/* Copyright 2007 Andrew Diakin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package ru.rpunet.webmops.web;

import java.util.List;

import net.sourceforge.stripes.action.DefaultHandler;
import net.sourceforge.stripes.action.ForwardResolution;
import net.sourceforge.stripes.action.Resolution;

import ru.rpunet.webmops.dao.PackageManager;
import ru.rpunet.webmops.model.Package;
import ru.rpunet.webmops.utils.WebmopsActionBean;

/**
 * @author Andrew Diakin
 *
 */
public class PackageLookupActionBean extends WebmopsActionBean {

	private String autoCompleteText;
	private PackageManager pm;
	private List<Package> packages;
	
	public String getAutoCompleteText() {
		return autoCompleteText;
	}
	
	public void setAutoCompleteText(String autoCompleteText) {
		this.autoCompleteText = autoCompleteText;
	}
	
	public List<Package> getPackages() {
		return packages;
	}
	
	public void setPackages(List<Package> packages) {
		this.packages = packages;
	}
	
	@DefaultHandler
	public Resolution findPackages() {
		pm = new PackageManager();
		packages = pm.findPackagesByName(autoCompleteText);
		
		return new ForwardResolution("/FindPackagesAjax.jsp");
	}
	
	
	
}
