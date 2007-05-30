/**
 * 
 */
package ru.rpunet.webmops.web;

import java.util.List;

import ru.rpunet.webmops.dao.PackageManager;
import ru.rpunet.webmops.model.File;
import ru.rpunet.webmops.model.Package;
import net.sourceforge.stripes.action.ActionBean;
import net.sourceforge.stripes.action.ActionBeanContext;
import net.sourceforge.stripes.action.DefaultHandler;
import net.sourceforge.stripes.action.ForwardResolution;
import net.sourceforge.stripes.action.RedirectResolution;
import net.sourceforge.stripes.action.Resolution;

/**
 * @author andrew
 *
 */
public class ShowFilesActionBean implements ActionBean {

	private ActionBeanContext context;
	
	private Long packageId;
	
	private Package pkg;
	
	private List<File> files;
	
		
	public ActionBeanContext getContext() {
		return this.context;
	}

	public void setContext(ActionBeanContext arg0) {
		this.context = arg0;
	}
	
	public Long getPackageId() {
		return this.packageId;
	}
	
	public void setPackageId(Long packageId) {
		this.packageId = packageId;
	}
	
	public List<File> getFiles() {
		return this.files;
	}
	
	public Package getPkg() {
		return this.pkg;
	}
	
	public int getFilesTotalNum() {
		return this.files.size();
	}
	
	@DefaultHandler
	public Resolution show() {
		
		PackageManager packageDAO = new PackageManager();
		pkg = packageDAO.findById(this.packageId);
		
		if ( pkg == null ) // oops...
			return new RedirectResolution("/PackageNotFound.jsp");
		
		files = pkg.getFiles();
				
		return new ForwardResolution("/FileList.jsp");
	}

}
