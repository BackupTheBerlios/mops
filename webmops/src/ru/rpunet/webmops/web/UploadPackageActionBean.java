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

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;

import net.sourceforge.stripes.action.DefaultHandler;
import net.sourceforge.stripes.action.DontValidate;
import net.sourceforge.stripes.action.FileBean;
import net.sourceforge.stripes.action.ForwardResolution;
import net.sourceforge.stripes.action.RedirectResolution;
import net.sourceforge.stripes.action.Resolution;
import net.sourceforge.stripes.validation.Validate;
import ru.rpunet.webmops.dao.PackageManager;
import ru.rpunet.webmops.model.Package;
import ru.rpunet.webmops.utils.WebmopsActionBean;

/**
 * Upload package to server
 * This implementation applies primary to old slacware packages, it 
 * doesn't validate anything, doesn't read install/slack-desc file on slackware packages
 * or install/package.xml in new mops packages. We don't use ValidateNestedProperties
 * because such things like md5, file name are not allowed to change from web.
 * 
 * TODO: add xml schema validation
 * TODO: add output repository property
 * 
 * @author Andrew Diakin
 *
 */
public class UploadPackageActionBean extends WebmopsActionBean {
	
	private Package pkg;
	
	@Validate(required=true, minlength=3)
	private String name;
	@Validate(required=true)
	private String version;
	@Validate(required=true)
	private String arch;
	@Validate(required=true)
	private String location;
	@Validate(required=true)
	private String md5;
	@Validate(required=true)
	private String build;
	
	@Validate(required=true)
	private FileBean packageFile;
	
	public FileBean getPackageFile() {
		return packageFile;
	}
	
	public void setPackageFile(FileBean packageFile) {
		this.packageFile = packageFile;
	}
	
	public String getArch() {
		return arch;
	}
	public void setArch(String arch) {
		this.arch = arch;
	}
	public String getBuild() {
		return build;
	}
	public void setBuild(String build) {
		this.build = build;
	}
	public String getLocation() {
		return location;
	}
	public void setLocation(String location) {
		this.location = location;
	}
	public String getMd5() {
		return md5;
	}
	public void setMd5(String md5) {
		this.md5 = md5;
	}
	public String getName() {
		return name;
	}
	public void setName(String name) {
		this.name = name;
	}
	public String getVersion() {
		return version;
	}
	public void setVersion(String version) {
		this.version = version;
	}
	
	@DontValidate
	@DefaultHandler
	public Resolution index() {
		return new ForwardResolution("/UploadPackageForm.jsp");
	}
	
	
	public Resolution save() {
		BufferedReader reader = null;
		BufferedWriter out = null;
		
		try {
			reader = new BufferedReader(new InputStreamReader(packageFile.getInputStream()));
			String line = null;
			// TODO: fix repository location
			out = new BufferedWriter(new FileWriter("/tmp/packages/" + packageFile.getFileName()));
			
			while ((line = reader.readLine()) != null) {
				out.write(line + "\n");
			}
			
			String filename = packageFile.getFileName();
			
			pkg = new Package();
			PackageManager packageDAO = new PackageManager();
			
			pkg.setName(name);
			pkg.setArch(arch);
			pkg.setBuild(build);
			pkg.setMd5(md5);
			pkg.setVersion(version);
			pkg.setLocation(location);
			pkg.setFileName(filename);
			
			packageDAO.makePersistent(pkg);
			
		} catch (IOException e) {
			e.printStackTrace();
		} finally {
			try {
				if (reader != null)
					reader.close();
				
				if ( out != null)
					out.close();
			} catch (Exception e) {
			}
		}
		
		return new RedirectResolution("/PackageInfo.action?id=" + pkg.getId());
	}
	
}
