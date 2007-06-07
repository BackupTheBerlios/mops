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

import net.sourceforge.stripes.action.DefaultHandler;
import net.sourceforge.stripes.action.ForwardResolution;
import net.sourceforge.stripes.action.HandlesEvent;
import net.sourceforge.stripes.action.RedirectResolution;
import net.sourceforge.stripes.action.Resolution;
import net.sourceforge.stripes.validation.LocalizableError;
import net.sourceforge.stripes.validation.Validate;
import net.sourceforge.stripes.validation.ValidationErrors;
import net.sourceforge.stripes.validation.ValidationMethod;
import ru.rpunet.webmops.utils.WebmopsActionBean;

/**
 * @author Andrew Diakin
 *
 */
public class SelectPackageTypeActionBean extends WebmopsActionBean {

	@Validate(required=true)
	private String packageType;
	
	public String getPackageType() {
		return this.packageType;
	}
	
	public void setPackageType(String packageType) {
		this.packageType = packageType;
	}
	
	@DefaultHandler
	public Resolution index() {
		return new ForwardResolution("/SelectPackageTYpe.jsp");
	}
	
	@HandlesEvent("selectType")
	public Resolution select() {
		if ( packageType.equalsIgnoreCase("SLACKWARE") ) {
			//return new RedirectResolution(null);
		} else if (packageType.equalsIgnoreCase("MPKG")) {
			return new RedirectResolution(ru.rpunet.webmops.web.UploadSlackwarePackageActionBean.class);
		}
		
		return null;
	}
	
	@ValidationMethod
	public void checkType(ValidationErrors errors) {
		if ( !packageType.equalsIgnoreCase("SLACKWARE") 
				|| !packageType.equalsIgnoreCase("MPKG")) {
			errors.add("packageType", new LocalizableError("unknownPackageType"));
		}
	}
}
