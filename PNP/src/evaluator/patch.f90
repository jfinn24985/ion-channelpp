!  -------------------------------------------------------------
!  ICC data
!
!  baksub
!     convert 'c' vector to 'h' vector by back-substituting
!     from the decomposed 'A' matrix
!  matrix
!     build the initial 'A' matrix and decompose it.

module patch
implicit none
private
  ! Maximum allocation size
  integer, private, parameter :: npchmx_=2048
  ! Whether to output debug_ information
  logical, private, parameter :: dbc_=.true.
  logical, private, parameter :: debug_=.false.
  ! file numbers
  integer, private, parameter :: fidlog=6
  integer, private, parameter :: fidpch=26
  integer, private, parameter :: fidamx=12
  ! 'A' matrix
  double precision, private, dimension(:,:), allocatable :: amx_
  ! the allocated size of the main patch arrays (<= npchmx_)
  integer, private :: npchsz_
  ! Patch x,y,z,area,ux,uy,uz,deps_ date [8 dbls] coords
  ! X coordinate of a patch
  double precision, private, dimension(:), allocatable :: prx_
  ! Y coordinate of a patch
  double precision, private, dimension(:), allocatable :: pry_
  ! Z coordinate of a patch
  double precision, private, dimension(:), allocatable :: prz_
  ! Surface area of a patch
  double precision, private, dimension(:), allocatable :: parea_
  ! X dimension of normal vector to centre of patch 
  double precision, private, dimension(:), allocatable :: pux_
  ! Y dimension of normal vector to centre of patch
  double precision, private, dimension(:), allocatable :: puy_
  ! Z dimension of normal vector to centre of patch
  double precision, private, dimension(:), allocatable :: puz_
  ! The effective dielectric constant on the outside of a patch
  double precision, private, dimension(:), allocatable :: deps_
  ! back substitution index vector
  integer, private, dimension(:), allocatable :: indx_
  ! pi_
  double precision, private :: pi_=3.14  !!INPUT!!
  ! water relative permittivity
  double precision, private :: epsw_=80.0  !!INPUT!!
  ! protein relative permittivity
  double precision, private :: epspr_=10.0  !!INPUT!!
  ! patch size factors
  double precision, private :: dxf_=1.6   !!INPUT!!
  double precision, private :: dxw_=1.6   !!INPUT!!
  ! number of patches
  integer, private :: npatch_ = 0
  ! patch integration factor
  integer, private :: nsub0_=10   !!INPUT!!
  ! Has the inverted 'A' matrix been read or created?
  logical, private :: irdamx_=.false.

  public :: baksub_, getnpch_, matrix_, patchsaves_, gaussbox_, totalarea_, writam_, writpc_
  public :: getx_, gety_, getz_, getux_, getuy_, getuz_, getarea_, getdep_
  type geomdf
     double precision, dimension(8,300) :: dfia,tta1,tta2
     double precision, dimension(6,300) :: dfil,zzl1,zzl2,rrl1,rrl2
     integer, dimension(8,300) :: nfia
     integer, dimension(6,300) :: nfil
     integer, dimension(20) :: ilsign
     double precision, dimension(8) :: za0,ra0,ra,ta1,ta2,uasign
     double precision, dimension(6) :: ulsign,zl1,rl1,zl2,rl2,tgalfa,alfa
     integer, dimension(8) :: nta
     integer, dimension(6) :: nzl,nrl
     integer :: narch,nline,nwall,nwall2,nunif,ncent
  end type geomdf

  interface
    pure elemental function dfeq(lhs,rhs)
      logical :: dfeq
      double precision, intent(in) :: lhs, rhs
    end function dfeq
    function next64(val)
      integer :: next64
      integer, intent(in) :: val
    end function next64
  end interface


interface str
  module procedure logstr
  module procedure fmtfln
end interface str

contains
  ! --------------------------------------------
  ! Give a text representation of a logical value
  !
  ! Converts a logical value into '.true.' or
  ! '.false.'
  pure subroutine logstr(a_bool, output)
    implicit none
    logical, intent(in) :: a_bool
    character(*), intent(inout) :: output
    character(*), parameter :: yes = ".true."
    character(*), parameter :: no = ".false."
    if (a_bool) then
      output=yes(1:max(1,min(len(output),len(yes))))
    else
      output=no(1:max(1,min(len(output),len(no))))
    endif
  end subroutine logstr

  ! ------------------------------------------------------------
  ! Method to pretty format floating point fields.
  !
  ! This method tries to select the best way to represent the number in
  ! the given width. Firstly one character is always reserved for the
  ! minus sign.  Numbers that can be nicely represented without exponent
  ! are formatted in fixed point, otherwise the numbers are formatted
  ! using an exponent.  Any decimal precision is reduced to the minimum
  ! representable by the type.  The formatted number is then checked for
  ! unnecessary zeros at the end of number, which are removed.
  !
  ! Examples
  ! -11.234 : Only as many decimal places as are non-zero are shown
  ! -0.3456 : Exponent is not used for numbers close to one
  ! -3.456E-001
  subroutine fmtfln(num, output)
    implicit none
    double precision, intent(in) :: num
    character(len=*), intent(inout) :: output
    interface 
      subroutine fmtfl2(val,str,sz)
        double precision, intent(in) :: val
        character(*), intent(inout) :: str
        integer, intent(in) :: sz
      end subroutine fmtfl2
    end interface
    call fmtfl2(num,output,len(output))
  end subroutine fmtfln



  ! Get the number of patches.
  function getnpch_()
    implicit none
    integer :: getnpch_
    getnpch_ = npatch_
  end function getnpch_

  function getx_(idx)
    implicit none
    double precision :: getx_
    integer, intent(in) :: idx
    getx_ = prx_( idx )
  end function getx_

  function gety_(idx)
    implicit none
    double precision :: gety_
    integer, intent(in) :: idx
    gety_ = pry_( idx )
  end function gety_

  function getz_(idx)
    implicit none
    double precision :: getz_
    integer, intent(in) :: idx
    getz_ = prz_( idx )
  end function getz_

  function getux_(idx)
    implicit none
    double precision :: getux_
    integer, intent(in) :: idx
    getux_ = pux_( idx )
  end function getux_

  function getuy_(idx)
    implicit none
    double precision :: getuy_
    integer, intent(in) :: idx
    getuy_ = puy_( idx )
  end function getuy_

  function getuz_(idx)
    implicit none
    double precision :: getuz_
    integer, intent(in) :: idx
    getuz_ = puz_( idx )
  end function getuz_

  function getarea_(idx)
    implicit none
    double precision :: getarea_
    integer, intent(in) :: idx
    getarea_ = parea_( idx )
  end function getarea_

  function getdep_(idx)
    implicit none
    double precision :: getdep_
    integer, intent(in) :: idx
    getdep_ = deps_( idx )
  end function getdep_

  ! --------------------------------
  ! calculate a gaussbox over ALL surfaces, so parea_ must be composed
  ! of closed surfaces
  !
  double precision function gaussbox_(h)
    implicit none
    double precision, dimension(npatch_), intent(in) :: h

    gaussbox_ = sum(parea_(1:npatch_)*h(1:npatch_)/deps_(1:npatch_))
  end function gaussbox_

  double precision function totalarea_()
    implicit none
    
    totalarea_ = sum(parea_(1:npatch_))
  end function totalarea_

  ! ------------------------
  ! Output gaussbox charge and area to fidooo
  !
  !
  ! astep : step number 1...
  !
  ! rchg = cumulative gauss sum/e0
  ! archg = cumulative abs(gauss)
  ! r2chg = cumulative gauss^2
  ! area = surface area for which gauss sum is calculated
  !
  ! chg = current gauss sum
  ! echg = avg rchg
  ! aechg = avg archg
  ! er2chg = avg r2chg
  !
  subroutine patchsaves_(fid,astep,h)
    implicit none
    integer, intent(in) :: fid,astep
    double precision, dimension(npatch_), intent(in) :: h
    double precision, save :: rchg = 0, archg = 0, r2chg = 0
    double precision, save :: area = 0
    double precision, save :: max_chg = 0
    double precision :: chg, echg, aechg, er2chg, abs_chg
    character(20) :: fltout

    if (area.eq.0) area = totalarea_()
    
    chg = gaussbox_(h)

    rchg = rchg + chg
    archg = archg + dabs(chg)
    r2chg = r2chg + chg**2

    echg = rchg/astep
    aechg = archg/astep
    er2chg = r2chg/astep
    
    abs_chg = dabs(chg)
    if (max_chg.lt.abs_chg) max_chg = abs_chg

    write(unit=fid, fmt='(" epspr_       = ",f10.5)')  epspr_
    call str(chg, fltout)
    write(unit=fid, fmt='(" gauss[a]    = ",A)')trim(adjustl(fltout))
    call str(echg, fltout)
    write(unit=fid, fmt=*)' <gauss>     = ',trim(adjustl(fltout))
    call str(aechg, fltout)
    write(unit=fid, fmt=*)' <|gauss|>   = ',trim(adjustl(fltout))
    call str(er2chg-echg**2, fltout)
    write(unit=fid, fmt=*)' Var(gauss)  = ',trim(adjustl(fltout))
    call str(er2chg-aechg**2, fltout)
    write(unit=fid, fmt=*)' Var(|gauss|)= ',trim(adjustl(fltout))
    call str(area, fltout)
    write(unit=fid, fmt=*)' area        = ',trim(adjustl(fltout))
    call str(max_chg, fltout)
    write(unit=fid, fmt=*)' max(|gauss|)= ', trim(adjustl(fltout))
  end subroutine patchsaves_
 
  ! Access functions
  ! ------------------------------------------------------------------
  ! Calculate the induced charge per unit area of a patch using
  ! the ICC protocol
  !
  ! The initialisation phase of the ICC protocol generate a solution
  ! matrix for the set of simultaneous equations representing the
  ! patches.  To generate the induced charges on all the patches
  ! we perform a back substitution on the solution matrix using
  ! 'hmat'.  This process is performed by an external Lapack
  ! routine.
  subroutine baksub_(hmat)
  implicit none
  integer :: n1,n2,info,nrhs,nx
  character(*), parameter :: trans="N"
  double precision, dimension(npchmx_), intent(inout) :: hmat
  external dgetrs
  if (dbc_) then
    if (.not.irdamx_) stop "Error: patch::baksub called with no inverted A matrix"
  endif
  info=0
  nrhs=1
  n1=npatch_
  n2=npatch_
  nx=npchsz_
  ! Use a LAPACK method to back substitute the rhs (b) 
  ! from the solution from ludcmp.
  call dgetrs (trans, n1, nrhs, amx_, nx, indx_, hmat, n2, info);
  if (0.ne.info) stop "Matrix back-substitution failed."

  end subroutine baksub_

  ! ------------------------------------------------------------------
  ! Generate the patch locations from the base geometry
  !
  !                 ^
  !                 ^ rl5
  !                 |
  !        <- zl3  >|
  !        ---------|---------
  !       /|        |  ^rl4   \
  !      / |        |  |   ____\
  !     |<-> = rlcurv  |  ^rl3  |
  !     |<--> = rlvest |  |_____|
  !     \   |       |  |  ^rl2  /
  !      \  |       |  |  |    /
  !       \ |       |  |  |   /
  !        \|<-zl1->|  |  |  /
  !         ----------------- 
  !           ^ rl1 |  |  |
  !      <-----zl2->|  |  |
  !   ________|_____|_ |__|______ z-axis
  !<<-------------->| zl4
  !
  subroutine defgrd(geodfn,zl1,rl1,rl4,rlvest,rlcurv)
  implicit none
  type (geomdf), intent(inout) :: geodfn
  double precision, intent(in) :: zl1,rl1,rl4,rlvest,rlcurv

  ! LOCALS
  integer, dimension(5,npchmx_) :: isurf
  double precision, dimension(8,300) :: dtta,ara,tac
  double precision, dimension(6,300) :: drl,arr,rlc,dzl,arl,zlc
  double precision, dimension(8) :: dta
  double precision, dimension(6) :: dll
  integer, dimension(5) :: nsurf
  integer ::  iii,is  ! Loop indices
  double precision, dimension(:,:), allocatable :: p__

  double precision :: rl2,rl3,zl2,zl3
  rl2 = rl1 + rlvest
  rl3 = rl4 - rlcurv
  zl2 = zl1 + rlvest
  zl3 = zl2 - rlcurv
  ! -----  Define doughnut geometry -----------------
  geodfn%narch=4
  geodfn%nline=2
  geodfn%nwall=2

  ! archs
  geodfn%za0(1)=  zl1
  geodfn%za0(2)=  zl3
  geodfn%za0(3)= -zl3
  geodfn%za0(4)= -zl1

  geodfn%ra0(1)=  rl2
  geodfn%ra0(2)=  rl3
  geodfn%ra0(3)=  rl3
  geodfn%ra0(4)=  rl2

  geodfn%ra(1)= rlvest
  geodfn%ra(2)= rlcurv
  geodfn%ra(3)= rlcurv
  geodfn%ra(4)= rlvest

  geodfn%ta1(1)= 3*pi_/2
  geodfn%ta1(2)= 0
  geodfn%ta1(3)= pi_/2
  geodfn%ta1(4)= pi_

  geodfn%ta2(1)= 2*pi_
  geodfn%ta2(2)= pi_/2
  geodfn%ta2(3)= pi_
  geodfn%ta2(4)= 3*pi_/2

  geodfn%uasign(1)=-1
  geodfn%uasign(2)=-1
  geodfn%uasign(3)=-1
  geodfn%uasign(4)=-1

  ! lines
  geodfn%zl1(1)= -zl1
  geodfn%rl1(1)=  rl1
  geodfn%zl2(1)=  zl1
  geodfn%rl2(1)=  rl1
  geodfn%alfa(1)   = 0
  geodfn%tgalfa(1) = 0

  geodfn%zl1(2)= -zl3
  geodfn%rl1(2)=  rl4
  geodfn%zl2(2)=  zl3
  geodfn%rl2(2)=  rl4
  geodfn%alfa(2)   = 0
  geodfn%tgalfa(2) = 0

  geodfn%ulsign(1)=-1
  geodfn%ulsign(2)=1
  geodfn%ulsign(3)=1
  geodfn%ulsign(4)=-1

  ! walls
  geodfn%zl1(3)=  zl2
  geodfn%rl1(3)=  rl2
  geodfn%zl2(3)=  zl2
  geodfn%rl2(3)=  rl3

  geodfn%zl1(4)= -zl2
  geodfn%rl1(4)=  rl2
  geodfn%zl2(4)= -zl2
  geodfn%rl2(4)=  rl3

  geodfn%alfa(5)=pi_/2
  geodfn%alfa(6)=pi_/2

  write(unit=fidlog,fmt=*)" ICC: Calculating tiles on the protein surface"
  write(unit=fidlog,fmt='(72("-"))')
  ! ------ go over 1st line: filter -----------------

  npatch_=0
  iii=0
  is=0
  nsurf=0
  allocate(p__(8,npchmx_))

  call gofilt

  npatch_=npatch_+iii
  write(unit=fidlog,fmt=*)" NUMBER OF TILES IN FILTER = ",iii
  write(unit=fidlog,fmt='(50("-"))')

  call goline

  write(unit=fidlog,fmt=*)" NUMBER OF TILES IN OUTER LINE = ",iii-npatch_
  write(unit=fidlog,fmt='(50("-"))')
  npatch_=iii

  call goarch

  write(unit=fidlog,fmt=*)" NUMBER OF TILES IN ARCHS = ",iii-npatch_
  write(unit=fidlog,fmt='(50("-"))')
  npatch_=iii

  call gowall

  write(unit=fidlog,fmt=*)" NUMBER OF TILES IN WALLS = ",iii-npatch_
  write(unit=fidlog,fmt='(50("-"))')
  write(unit=fidlog,fmt=*)" NUMBER OF TILES TOTAL    = ",iii
  write(unit=fidlog,fmt='(72("-"))')
  npatch_=iii

  !----------------------------------------------------------------------
  ! set npchsz_ and allocate and copy data to patch main arrays
  npchsz_=next64(npatch_)

  ! If prx_ is already defined then this is second call to this method.
  if (.not.allocated(prx_)) then
    allocate(prx_(npchsz_))
    allocate(pry_(npchsz_))
    allocate(prz_(npchsz_))
    allocate(parea_(npchsz_))
    allocate(pux_(npchsz_))
    allocate(puy_(npchsz_))
    allocate(puz_(npchsz_))
    allocate(deps_(npchsz_))
  endif

  prx_(1:npatch_)=p__(1,1:npatch_)
  pry_(1:npatch_)=p__(2,1:npatch_)
  prz_(1:npatch_)=p__(3,1:npatch_)
  parea_(1:npatch_)=p__(4,1:npatch_)
  pux_(1:npatch_)=p__(5,1:npatch_)
  puy_(1:npatch_)=p__(6,1:npatch_)
  puz_(1:npatch_)=p__(7,1:npatch_)
  deps_(1:npatch_)=p__(8,1:npatch_)

  deallocate(p__)

  contains

  ! For lines close and parallel to the z axis
  !
  subroutine gofilt
  implicit none
  integer ::  i,j,k  ! Loop indices
  double precision :: rc,fik,cir
  i=1    
  write(unit=fidlog,fmt='(1X,I1,A)')i,"th line"
  write(unit=fidlog,fmt='(A20,2(1X,F7.2))')"start (z,r) =",geodfn%zl1(i),geodfn%rl1(i)
  write(unit=fidlog,fmt='(A20,2(1X,F7.2))')"end (z,r) =",geodfn%zl2(i),geodfn%rl2(i)
  if (dbc_) then
     if (.not.dfeq(geodfn%rl1(i),geodfn%rl2(i))) then
        stop "Line not parallel to z-axis"
     endif
  endif
  dll(i)=(geodfn%zl2(i)-geodfn%zl1(i))
  write(unit=fidlog,fmt='(A20,1X,F7.2)')"length (z) =",dll(i)

  ! --- loop over xz coordinate ---------------------
  geodfn%nzl(i)=max(10,int(dll(i)/dxf_)+1)
  write(unit=fidlog,fmt='(A20,1X,I4)')"mesh size (z) =",geodfn%nzl(i)
  i3y5r5: do j=1,geodfn%nzl(i)
  !    write(unit=fidlog,fmt=*)j,"th ring"
    dzl(i,j)=(geodfn%zl2(i)-geodfn%zl1(i))/dble(geodfn%nzl(i))
    o3j9m0: if (j.eq.1) then
      geodfn%zzl1(i,j)=geodfn%zl1(i)
      geodfn%zzl2(i,j)=geodfn%zl1(i)+dzl(i,j)
    else o3j9m0
      geodfn%zzl1(i,j)=geodfn%zzl2(i,j-1)
      geodfn%zzl2(i,j)=geodfn%zzl1(i,j)+dzl(i,j)
    endif o3j9m0

    arl(i,j)=(geodfn%zzl2(i,j)-geodfn%zzl1(i,j))
    zlc(i,j)=(geodfn%zzl2(i,j)+geodfn%zzl1(i,j))/2

  ! --- loop over phi angle ------------------------
    rc=geodfn%rl1(i)
    cir=2*pi_*rc
    geodfn%nfil(i,j)=max(16,int(cir/dxf_)+1)
    geodfn%dfil(i,j)=2*pi_/dble(geodfn%nfil(i,j))

    write(unit=fidlog,fmt='(A20,1X,I4)')"mesh size (r) =",geodfn%nfil(i,j)

    b3a4l6: do k=1,geodfn%nfil(i,j)
      iii=iii+1
      is=is+1
      nsurf(1)=nsurf(1)+1
      isurf(1,nsurf(1))=is
      fik=(k-0.5D0)*geodfn%dfil(i,j)
      p__(1,iii)=rc*dcos(fik)
      p__(2,iii)=rc*dsin(fik)
      p__(3,iii)=zlc(i,j)
      if (dbc_) then
        if (isnan(p__(1,iii))) stop "px is NaN"
        if (isnan(p__(2,iii))) stop "py is NaN"
        if (isnan(p__(3,iii))) stop "pz is NaN"
      endif
      p__(5,iii)=geodfn%ulsign(i)*dcos(fik)
      p__(6,iii)=geodfn%ulsign(i)*dsin(fik)
      p__(7,iii)=-0.D0
      if (dbc_) then
        if (isnan(p__(5,iii))) stop "ux is NaN"
        if (isnan(p__(6,iii))) stop "uy is NaN"
        if (isnan(p__(7,iii))) stop "uz is NaN"
      endif
      ! zlength = arl  rlength = 2.pi_.r/nfil <==> dfil.r
      p__(4,iii)=arl(i,j)*geodfn%dfil(i,j)*rc
      if (dbc_) then
        if (p__(4,iii).le.0.0) stop "area is negative"
        if (isnan(p__(4,iii))) stop "area is NaN"
        if (isnan(arl(i,j))) stop "arl is NaN"
      endif
      p__(8,iii)=2*(epsw_-epspr_)/(epspr_+epsw_)
    enddo b3a4l6
  enddo i3y5r5
  write(unit=fidlog,fmt="(50('-'))")
  end subroutine gofilt

  ! For lines distant and parallel to the z-axis
  subroutine goline
  implicit none
  integer ::  i,j,k  ! Loop indices
  double precision :: rc,dx,fik,cir
  ! ------ go over outer cylinder -------------------

  p6o0d0: do i=2,geodfn%nline
    write(unit=fidlog,fmt='(1X,I1,A)')i,"th line"
    write(unit=fidlog,fmt='(A20,1X,F7.2,1X,F7.2)')"start (z,r) =",geodfn%zl1(i),geodfn%rl1(i)
    write(unit=fidlog,fmt='(A20,1X,F7.2,1X,F7.2)')"end (z,r) =",geodfn%zl2(i),geodfn%rl2(i)
    dll(i)=(geodfn%zl2(i)-geodfn%zl1(i))
    if (dbc_) then
       if (.not.dfeq(geodfn%rl1(i),geodfn%rl2(i))) then
          stop "Line not parallel to z-axis"
       endif
    endif
    write(unit=fidlog,fmt='(A20,1X,F7.2)')"length (z) =",dll(i)

  ! --- loop over z coordinate ---------------------
    dx=dxw_*dsqrt(geodfn%rl2(i))
    geodfn%nzl(i)=max(4,int(dll(i)/dx)+1)
    write(unit=fidlog,fmt='(A20,1X,I4)')"mesh size (z) =",geodfn%nzl(i)
    p3o5e7: do j=1,geodfn%nzl(i)
  !      write(unit=fidlog,fmt=*)j,"th ring"
      dzl(i,j)=(geodfn%zl2(i)-geodfn%zl1(i))/dble(geodfn%nzl(i))
      e2g3j7: if (j.eq.1) then
        geodfn%zzl1(i,j)=geodfn%zl1(i)
        geodfn%zzl2(i,j)=geodfn%zl1(i)+dzl(i,j)
      else e2g3j7
        geodfn%zzl1(i,j)=geodfn%zzl2(i,j-1)
        geodfn%zzl2(i,j)=geodfn%zzl1(i,j)+dzl(i,j)
      endif e2g3j7
      arl(i,j)=(geodfn%zzl2(i,j)-geodfn%zzl1(i,j))
      zlc(i,j)=(geodfn%zzl2(i,j)+geodfn%zzl1(i,j))/2

  ! --- loop over phi angle ------------------------
      rc=geodfn%rl1(i)
      cir=2*pi_*rc
      geodfn%nfil(i,j)=max(16,int(cir/dx)+1)
      geodfn%dfil(i,j)=2*pi_/dble(geodfn%nfil(i,j))
      write(unit=fidlog,fmt='(A20,1X,I4)')"mesh size (r) =",geodfn%nfil(i,j)

      v4z2z1: do k=1,geodfn%nfil(i,j)
        iii=iii+1
        is=is+1
        nsurf(2)=nsurf(2)+1
        isurf(2,nsurf(2))=is
        fik=(k-0.5D0)*geodfn%dfil(i,j)
        p__(1,iii)=rc*dcos(fik)
        p__(2,iii)=rc*dsin(fik)
        p__(3,iii)=zlc(i,j)
        if (dbc_) then
          if (isnan(p__(1,iii))) stop "px is NaN"
          if (isnan(p__(2,iii))) stop "py is NaN"
          if (isnan(p__(3,iii))) stop "pz is NaN"
        endif
        p__(5,iii)=geodfn%ulsign(i)*dcos(fik)
        p__(6,iii)=geodfn%ulsign(i)*dsin(fik)
        p__(7,iii)=-0.D0
        if (dbc_) then
          if (isnan(p__(5,iii))) stop "ux is NaN"
          if (isnan(p__(6,iii))) stop "uy is NaN"
          if (isnan(p__(7,iii))) stop "uz is NaN"
        endif
        ! zlength = arl  rlength = 2.pi_.r/nfil <==> dfil.r
        p__(4,iii)=arl(i,j)*geodfn%dfil(i,j)*rc
        if (dbc_) then
          if (p__(4,iii).le.0.0) stop "area is negative"
          if (isnan(p__(4,iii))) stop "area is NaN"
          if (isnan(arl(i,j))) stop "arl is NaN"
        endif
        p__(8,iii)=2*(epsw_-epspr_)/(epspr_+epsw_)
      enddo v4z2z1
    enddo p3o5e7
    write(unit=fidlog,fmt="(50('-'))")
  enddo p6o0d0
  end subroutine goline

  !
  ! Jim Fonseca's method for calculating patches on an arch
  !
  subroutine goarch
  implicit none
  double precision :: dla
  integer ::  i,j,k  ! Loop indices
  double precision :: dx,rc,fik,szaml,cir
  c1q4v0: do i=1,geodfn%narch
  
    write(unit=fidlog,fmt='(1X,I1,A)')i,"th arch"
    write(unit=fidlog,fmt='(A20,1X,F7.2,1X,F7.2)')"center (z,r) =",geodfn%za0(i),geodfn%ra0(i)
    write(unit=fidlog,fmt='(A20,1X,F7.2)')"radius =",geodfn%ra(i)
    write(unit=fidlog,fmt='(A20,1X,F7.2,1X,F7.2)')"angles (begin,end) =",geodfn%ta1(i),geodfn%ta2(i)
    dta(i)=geodfn%ta2(i)-geodfn%ta1(i)
    write(unit=fidlog,fmt='(A20,1X,F7.2)')"arc =",dta(i)   
    dla=geodfn%ra(i)*dta(i)
    write(unit=fidlog,fmt='(A20,1X,F7.2)')"length (z) =",dla
    dx=dxw_*dsqrt(geodfn%ra0(i))

  ! loop over theta angle 
  !theta is measured along channel axis
    geodfn%nta(i) = max(4,int(dla/dx)+1)
        
    write(unit=fidlog,fmt='(A20,1X,I4)')"mesh size (z=arc) =",geodfn%nta(i)
        
  !loop over arc        
    do j=1,geodfn%nta(i)
  !    write(unit=fidlog,fmt=*)j,"th ring"
  !each section of the arc has length dtta(arcnumber,ring)
      dtta(i,j)=dta(i)/dble(geodfn%nta(i))
      if (j.eq.1) then
  ! if it's the first ring, set to beginning theta of the arc keep
  ! in mind it seems that we are counting arcs from right to left,
  ! since we start at theta1, hopefully this doesn't make much of a
  ! difference (i.e. doesn't matter if go ta1 to ta2 or vice versa)
        geodfn%tta1(i,j)=geodfn%ta1(i)
        geodfn%tta2(i,j)=geodfn%ta1(i)+dtta(i,j)
      else
        geodfn%tta1(i,j)=geodfn%tta2(i,j-1)
        geodfn%tta2(i,j)=geodfn%tta1(i,j)+dtta(i,j)
      endif
  !find the center theta (which is not just the average of tta1,tta2)
      ara(i,j)=geodfn%ra0(i)*dtta(i,j) &
        -geodfn%ra(i)*(dcos(geodfn%tta2(i,j))-dcos(geodfn%tta1(i,j)))
      szaml=0.5*geodfn%ra0(i)*(geodfn%tta2(i,j)**2-geodfn%tta1(i,j)**2) &
        - geodfn%ra(i)*(geodfn%tta2(i,j)*dcos(geodfn%tta2(i,j))         &
               -geodfn%tta1(i,j)*dcos(geodfn%tta1(i,j)))+               &
        geodfn%ra(i)*(dsin(geodfn%tta2(i,j))-dsin(geodfn%tta1(i,j)))
      tac(i,j)=szaml/ara(i,j)
      rc=geodfn%ra0(i)+geodfn%ra(i)*dsin(tac(i,j))
  !circumference of this ring            
      cir=2*pi_*rc
      geodfn%nfia(i,j)=max(16,int(cir/dx)+1)
      geodfn%dfia(i,j)=2*pi_/dble(geodfn%nfia(i,j))
      write(unit=fidlog,fmt='(A20,3(1X,F7.2))')"theta (lo,mid,hi) =",geodfn%tta1(i,j),tac(i,j),geodfn%tta2(i,j)
      write(unit=fidlog,fmt='(A20,1X,I4)')"mesh size (r) =",geodfn%nfia(i,j)

  !loop over all the slices of phi            
      z9c4a8: do k=1,geodfn%nfia(i,j)
        iii=iii+1
        is=is+1
        y4h5p2: if (i.eq.1.or.i.eq.4) then
          nsurf(1)=nsurf(1)+1
          isurf(1,nsurf(1))=is
        else y4h5p2
          nsurf(2)=nsurf(2)+1
          isurf(2,nsurf(2))=is
        endif y4h5p2
        fik=(k-0.5D0)*geodfn%dfia(i,j)
        if (dbc_) then
          if (isnan(fik)) stop "fik is NaN"
        endif
        p__(1,iii)=rc*dcos(fik)
        p__(2,iii)=rc*dsin(fik)
        p__(3,iii)=geodfn%za0(i)+geodfn%ra(i)*dcos(tac(i,j))
        if (dbc_) then
          if (isnan(p__(1,iii))) stop "px is NaN"
          if (isnan(p__(2,iii))) stop "py is NaN"
          if (isnan(p__(3,iii))) stop "pz is NaN"
        endif
        p__(5,iii)=-geodfn%uasign(i)*dsin(tac(i,j))*dcos(fik)
        p__(6,iii)=-geodfn%uasign(i)*dsin(tac(i,j))*dsin(fik)
        p__(7,iii)=-geodfn%uasign(i)*dcos(tac(i,j))
        if (dbc_) then
          if (isnan(p__(5,iii))) stop "ux is NaN"
          if (isnan(p__(6,iii))) stop "uy is NaN"
          if (isnan(p__(7,iii))) stop "uz is NaN"
        endif
        p__(4,iii)=geodfn%ra(i)*ara(i,j)*geodfn%dfia(i,j)
        if (dbc_) then
          if (p__(4,iii).le.0.0) stop "area is negative"
          if (isnan(p__(4,iii))) stop "area is NaN"
          if (isnan(ara(i,j))) stop "ara is NaN"
        endif
        p__(8,iii)=2*(epsw_-epspr_)/(epspr_+epsw_)
      enddo z9c4a8
    enddo
    write(unit=fidlog,fmt="(50('-'))")
  enddo c1q4v0
  ! end of loop over archs 
  end subroutine goarch

  subroutine gowall
  implicit none
  integer ::  i,j,k  ! Loop indices
  double precision :: cir,ratio,rc,dfiu,fik,szaml
  integer :: nfiu
  
  ! ------ go over last walls of protein --------

  b0j3s2: do i=geodfn%nline+1,geodfn%nline+geodfn%nwall

    write(unit=fidlog,fmt='(1X,I1,A)')i,"th line/wall"
    write(unit=fidlog,fmt='(A20,2(1X,F7.2))')"start (z,r) =",geodfn%zl1(i),geodfn%rl1(i)
    write(unit=fidlog,fmt='(A20,2(1X,F7.2))')"end (z,r) =",geodfn%zl2(i),geodfn%rl2(i)
    dll(i)=geodfn%rl2(i)-geodfn%rl1(i)
    write(unit=fidlog,fmt='(A20,1X,F7.2)')"length (r) =",dll(i)

  ! --- loop over the rest with increasing tiles ---
    cir=2*pi_*(geodfn%rl1(i)+dll(i)/2)
    nfiu=int(cir/dxf_)+1
    dfiu=2*pi_/dble(nfiu)
    ratio=(1+dfiu/2)/(1-dfiu/2)
    ! write(unit=fidlog,fmt=*)" Ratio old = ",ratio
    geodfn%nrl(i)=int(dlog(geodfn%rl2(i)/geodfn%rl1(i))/dlog(ratio))+1
    ! write(unit=fidlog,fmt=*)" N r for increasing tiles = ",geodfn%nrl(i)
    ratio=(geodfn%rl2(i)/geodfn%rl1(i))**(1/dble(geodfn%nrl(i)))
    ! write(unit=fidlog,fmt=*)" Ratio new = ",ratio
    write(unit=fidlog,fmt='(A20,1X,I4)')"mesh size (r)",geodfn%nrl(i)
    geodfn%rrl1(i,1)=geodfn%rl1(i)
    w8w7s2: do j=1,geodfn%nrl(i)
      geodfn%dfil(i,j)=dfiu
      geodfn%rrl2(i,j)=geodfn%rrl1(i,j)*ratio
      ! write(unit=fidlog,fmt=*)j,"th ring"
      drl(i,j)=geodfn%rrl2(i,j)-geodfn%rrl1(i,j)
      ! write(unit=fidlog,fmt=*)"   dr     = ",drl(i,j)
      arr(i,j)=0.5D0*(geodfn%rrl2(i,j)**2-geodfn%rrl1(i,j)**2)
      szaml=(1.D0/3.D0)*(geodfn%rrl2(i,j)**3-geodfn%rrl1(i,j)**3)
      rlc(i,j)=szaml/arr(i,j)

  ! --- loop over phi angle --------------------
      rc=rlc(i,j)
      geodfn%nfil(i,j)=nfiu
      geodfn%dfil(i,j)=dfiu
      write(unit=fidlog,fmt='(A20,3(1X,F7.2))')"r (lo,mid,hi) =",geodfn%rrl1(i,j),rlc(i,j),geodfn%rrl2(i,j)
      write(unit=fidlog,fmt='(A20,1X,I4)')"mesh size (r) =",geodfn%nfil(i,j)
      i6o4l5: do k=1,geodfn%nfil(i,j)
        iii=iii+1
        is=is+1
        nsurf(2)=nsurf(2)+1
        isurf(2,nsurf(2))=is
        fik=(k-0.5D0)*geodfn%dfil(i,j)
        p__(1,iii)=rc*dcos(fik)
        p__(2,iii)=rc*dsin(fik)
        p__(3,iii)=geodfn%zl1(i)
        p__(5,iii)=0
        p__(6,iii)=0
        p__(7,iii)=geodfn%ulsign(i)
        p__(4,iii)=arr(i,j)*geodfn%dfil(i,j)
        p__(8,iii)=2*(epsw_-epspr_)/(epspr_+epsw_)
      enddo i6o4l5
      geodfn%rrl1(i,j+1)=geodfn%rrl2(i,j)
    enddo w8w7s2
    write(unit=fidlog,fmt="(50('-'))")
  enddo b0j3s2

  end subroutine gowall

  end subroutine defgrd

  ! ------------------------------------------------------------------
  ! Compute the A matrix
  !
  ! NOTE: The saved matrix does not adjust the i=j or multiply by deps_

  subroutine matrix_(pi,dxf,dxw,epsw,epspr,nsub0,zl1,rl1,rl4,rlvest,rlcurv)
  implicit none
  double precision, intent(in) :: pi,dxf,dxw,epsw,epspr
  integer, intent(in) :: nsub0     ! patch number (input) parameters
  double precision, intent(in) :: zl1,rl1,rl4,rlvest,rlcurv
  type (geomdf), allocatable :: geodfn

  external dgetrf
  ! LOCALS
  double precision fi1,fi2    ! derived patch size params
  double precision  aij       ! integrator output
  ! LOCALS
  integer i,j,k ! loop indices
  integer nsub     ! patch number (input) parameters
  integer ipatch,jpatch              ! 'A' matrix indices
  integer np10  ! counter for percent-complete output
  character*(*) fmt101
  parameter (fmt101='(2X,I8)')
  ! LOCAL
  integer info,npch2

  pi_ = pi
  dxf_ = dxf
  dxw_ = dxw
  epsw_ = epsw
  epspr_ = epspr
  nsub0_ = nsub0

  if (dbc_) then
    if (irdamx_) stop "patch::matrix called when inverted matrix exists"
  endif
  ! if not aa then rdmtrx
  allocate(geodfn)

  ! -----  Define geometry ------------------------------
  call defgrd(geodfn,zl1,rl1,rl4,rlvest,rlcurv)

  ! Npatch defined, allocate memory
  call f4s3s6
  ! ----  Fill matrix -----------------------------------

  write(unit=fidlog,fmt=*)" Filling matrix, patience"
  np10=npatch_/10

  e2o3c9: do ipatch=1,npatch_

    if (mod(ipatch,np10).eq.0) write(unit=fidlog,fmt='(i3," %")')10*ipatch/np10

    jpatch=0

    h8s9k7: do i=1,geodfn%nline
      x4x9c4: do j=1,geodfn%nzl(i)
        r5y4j7: do k=1,geodfn%nfil(i,j)
          jpatch=jpatch+1
          p8q0w3: if (ipatch.eq.jpatch) then
  !           if this patch is 'seeing' itself, we need lots of tiny patches
            nsub=5*nsub0_
          else p8q0w3
            nsub=nsub0_
          endif p8q0w3
  !         fi1/2 is start/end of distance (in radians) around the phi angle
          fi1=(k-1)*geodfn%dfil(i,j)
          fi2=k*geodfn%dfil(i,j)
          call intlin(nsub,geodfn%zl1(i),geodfn%rl1(i),geodfn%tgalfa(i),geodfn%zzl1(i,j),geodfn%zzl2(i,j),fi1,fi2,ipatch,aij)
          amx_(ipatch,jpatch)=aij/(4*pi_)
        enddo r5y4j7
      enddo x4x9c4
    enddo h8s9k7

    q1w3h2: do i=1,geodfn%narch
      h8l6c7: do j=1,geodfn%nta(i)
        f8g1o9: do k=1,geodfn%nfia(i,j)
          jpatch=jpatch+1
          k4v5z9: if (ipatch.eq.jpatch) then
  !           if this patch is 'seeing' itself, we need lots of tiny patches
            nsub=5*nsub0_
          else k4v5z9
            nsub=nsub0_
          endif k4v5z9
  !         fi1/2 is start/end of distance (in radians) around the phi angle
          fi1=(k-1)*geodfn%dfia(i,j)
          fi2=k*geodfn%dfia(i,j)
  !          write(unit=fidlog,fmt=*)ipatch,jpatch,nsub,geodfn%za0(i),geodfn%ra0(i),geodfn%ra(i),geodfn%tta1(i,j),
  ! :          geodfn%tta2(i,j),fi1,fi2,uasign(i)
          call intarc(nsub,geodfn%za0(i),geodfn%ra0(i),geodfn%ra(i),geodfn%tta1(i,j),geodfn%tta2(i,j),fi1,fi2,ipatch,aij)
          amx_(ipatch,jpatch)=aij/(4*pi_)
        enddo f8g1o9
      enddo h8l6c7
    enddo q1w3h2

    l3d7u8: do i=geodfn%nline+1,geodfn%nline+geodfn%nwall
      v0y7z9: do j=1,geodfn%nrl(i)
        a9e7o1: do k=1,geodfn%nfil(i,j)
          jpatch=jpatch+1
          x2t2t9: if (ipatch.eq.jpatch) then
  !           if this patch is 'seeing' itself, we need lots of tiny patches
            nsub=5*nsub0_
          else x2t2t9
            nsub=nsub0_
          endif x2t2t9
  !         fi1/2 is start/end of distance (in radians) around the phi angle
          fi1=(k-1)*geodfn%dfil(i,j)
          fi2=k*geodfn%dfil(i,j)
          call intwll(nsub,geodfn%zl1(i),geodfn%rrl1(i,j),geodfn%rrl2(i,j),fi1,fi2,ipatch,aij)
          amx_(ipatch,jpatch)=aij/(4*pi_)
        enddo a9e7o1
      enddo v0y7z9
    enddo l3d7u8
  enddo e2o3c9

  deallocate(geodfn)
  if (debug_) then
    call dumpam( "amx0.dat" )
  endif

  ! -----------------------------------------------------
  ! Update A matrix with deps_ etc
  forall (i=1:npatch_) amx_(i,1:npatch_)=deps_(i)*amx_(i,1:npatch_)
  indx_=0
  forall (i=1:npatch_) amx_(i,i)=amx_(i,i)+1

  if (debug_) then
    call dumpam( "amx1.dat" )
  endif

  ! -----------------------------------------------------
  ! Do LU decomposition of the A matrix
  ! call ludcmp
  write(unit=fidlog,fmt=*)"LU decomposing the 'A' matrix (using BLAS)"
  npch2=npchsz_
  info=0
  call dgetrf(npatch_, npchsz_, amx_, npch2, indx_, info)
  if (debug_) write(unit=fidlog,fmt=*)"! Matrix has been inverted, info = ",info
  if (0.ne.info) stop "Matrix inversion failed, no save."

  irdamx_=.true.

  contains

  !----------------------------------------------------------------------
  ! Private method to allocate second half of patch system
  ! 
  subroutine f4s3s6
  implicit none

  if (.not.allocated(amx_)) then
    allocate(amx_(npchsz_,npchsz_),indx_(npchsz_))
    amx_=0
    indx_=0
  endif
  end subroutine f4s3s6
 
  ! -----------------------------------------------------
  ! Integrate a patch on an arc
  subroutine intarc(nsub,z0,r0,r,t1,t2,fi1,fi2,ii,aij)
  implicit none
  integer nsub,ii
  double precision z0,r0,r,t1,t2,fi1,fi2,aij

  ! LOCALS
  double precision ar,arel,cosfij,costtc,dfi,dfisub,dt,dtsub
  double precision fij,pxi,pxj,pxij,pyi,pyj,pyij,pzi,pzj,pzij
  double precision rij,rij3,rijsq,sinfij,sinttc,rc,szaml,tt1,tt2
  double precision ttc,uxi,uyi,uzi
  integer i,j ! loop indices

  ar=0
  pxi=prx_(ii)
  pyi=pry_(ii)
  pzi=prz_(ii)
  uxi=pux_(ii)
  uyi=puy_(ii)
  uzi=puz_(ii)

  dt=t2-t1
  dtsub=dt/dble(nsub)
  dfi=fi2-fi1
  dfisub=dfi/dble(nsub)

  aij=0
  if (dfeq(0.0D0,dfi)) stop "error in intarc, dfi = 0"
  ! --- double loop over subtiles ----------------------
  ! --- loop over theta angle --------------------------

  o3f5x0: do i=1,nsub
    r3o7r0: if (dfeq(0.0D0,dtsub)) then
      sinttc=0
      costtc=1
    else   r3o7r0
      tt1=t1+(i-1)*dtsub
      tt2=tt1+dtsub
      ar=r0*dtsub-r*(dcos(tt2)-dcos(tt1))
      szaml=0.5D0*r0*(tt2**2-tt1**2)-r*(tt2*dcos(tt2)-tt1*dcos(tt1))+r*(dsin(tt2)-dsin(tt1))
      ttc=szaml/ar
      if (dbc_) then
        if (isnan(ttc)) stop "error in intarc, ttc is NaN"
      endif
      sinttc=dsin(ttc)
      costtc=dcos(ttc)
    endif r3o7r0
    rc=r0+r*sinttc
    arel=r*ar*dfisub
    if (dbc_) then
      if (isnan(rc)) stop "error in intarc, rc is NaN"
      if (isnan(arel)) stop "error in intarc, arel is NaN"
    endif
  ! --- loop over phi angle ----------------------------
    pzj=z0+r*costtc

    y6p7q8: do j=1,nsub
      fij=fi1+(j-0.5D0)*dfisub
      cosfij=dcos(fij)
      sinfij=dsin(fij)
      pxj=rc*cosfij
      pyj=rc*sinfij
      pxij=pxj-pxi
      pyij=pyj-pyi
      pzij=pzj-pzi
      rijsq=pxij*pxij+pyij*pyij+pzij*pzij
      rij=dsqrt(rijsq)
      rij3=rijsq*rij
      aij=aij+(pxij*uxi+pyij*uyi+pzij*uzi)*arel/rij3
    enddo y6p7q8
  enddo o3f5x0
  if (dbc_) then
    if (isnan(aij)) stop "error in intarc, aij is NaN"
  endif
  end subroutine intarc

  ! -----------------------------------------------------
  ! Integrate a patch on cylindrical surface

  subroutine intlin(nsub,z0,r0,talfa,z1,z2,fi1,fi2,ii,aij)
  implicit none
  integer nsub,ii
  double precision z0,r0,talfa,z1,z2,fi1,fi2,aij
  ! LOCALS
  double precision ar,arel,cosfij,dfi,dfisub,dz,dzsub
  double precision fij,pxi,pxj,pxij,pyi,pyj,pyij,pzi,pzj,pzij
  double precision rij,rij3,rijsq,sinfij,rc,szaml,zz1,zz2
  double precision zc,uxi,uyi,uzi,calfa
  integer i,j ! loop indices

  pxi=prx_(ii)
  pyi=pry_(ii)
  pzi=prz_(ii)
  uxi=pux_(ii)
  uyi=puy_(ii)
  uzi=puz_(ii)

  ! calfa is cos of alfa
  calfa=1/dsqrt(1+talfa**2)
  ! z2 and z1 are end and begin of this particular ring
  dz=z2-z1
  ! so split up each ring into pieces (10)
  dzsub=dz/dble(nsub)
  ! also around the phi angle
  dfi=fi2-fi1
  dfisub=dfi/dble(nsub)

  aij=0
  ! gonna sum up aij for each patch
  ! will sum over tiny patches (nsub*nsub tiny patches)

  ! --- double loop over subtiles ------
  ! --- loop over theta angle ----------
  ! so each patch is going to have nsub*nsub tiny patches
  i2a3m1: do i=1,nsub
      zz1=z1+(i-1)*dzsub
      zz2=zz1+dzsub
  ! get the centroid in this weird way i can't figure out
  ! r0 is radius if line at beginning (left) side
  ! z0 beginning (left) of line
  ! talfa is tangent of the slope of the line
  ! dzsub is spacing between each tiny patch
  ! zz2 is right side of tinypatch
  ! zz1 is left side of tinypatch
    ar=(r0-z0*talfa)*dzsub+talfa*0.5D0*(zz2**2-zz1**2)
    szaml=0.5D0*(r0-z0*talfa)*(zz2**2-zz1**2)+talfa*(1.D0/3.D0)*(zz2**3-zz1**3)
    zc=szaml/ar
    rc=r0-z0*talfa+zc*talfa
    arel=rc/calfa

  ! --- loop over phi angle ------------

    r3g2u7: do j=1,nsub
  ! loop over tiny patches around phi angle
  ! fij is center of tiny patch around phi angle
      fij=fi1+(j-0.5D0)*dfisub
      cosfij=dcos(fij)
      sinfij=dsin(fij)
  ! pxj,pyz, and pzj are centroid of tiny patch
      pxj=rc*cosfij
      pyj=rc*sinfij
      pzj=zc
  ! pxi,pyi,pzi is center of patch
      pxij=pxj-pxi
      pyij=pyj-pyi
      pzij=pzj-pzi
      rijsq=pxij*pxij+pyij*pyij+pzij*pzij
  ! rij is the distance from the patch centroid to the tiny patch
      rij=dsqrt(rijsq)
      rij3=rijsq*rij
      aij=aij+(pxij*uxi+pyij*uyi+pzij*uzi)*arel*dzsub*dfisub/rij3
    enddo r3g2u7
  enddo i2a3m1
  if (dbc_) then
    if (isnan(aij)) stop "error in intlin, aij is NaN"
  endif
  end subroutine intlin

  ! -----------------------------------------------------
  ! Integrate patch on a disk surface
  subroutine intwll(nsub,z0,r1,r2,fi1,fi2,ii,aij)
  implicit none
  integer nsub,ii
  double precision z0,r1,r2,fi1,fi2,aij

  ! LOCALS
  double precision ar,arel,dfi,dfisub,dr,drsub
  double precision fij,pxi,pxj,pxij,pyi,pyj,pyij,pzi,pzj,pzij
  double precision rij,rij3,rijsq,rc,szaml,rr1,rr2,uxi,uyi,uzi
  integer i,j ! loop indices

  pxi=prx_(ii)
  pyi=pry_(ii)
  pzi=prz_(ii)
  uxi=pux_(ii)
  uyi=puy_(ii)
  uzi=puz_(ii)

  dr=r2-r1
  drsub=dr/dble(nsub)
  dfi=fi2-fi1
  dfisub=dfi/dble(nsub)

  aij=0
  ! --- double loop over subtiles ------
  ! --- loop over theta angle ----------

  q1e3h4: do i=1,nsub
    rr1=r1+(i-1)*drsub
    rr2=rr1+drsub
    ar=0.5D0*(rr2**2-rr1**2)
    szaml=(1.D0/3.D0)*(rr2**3-rr1**3)
    rc=szaml/ar
    arel=rc

  ! --- loop over phi angle ------------

    f1l0h6: do j=1,nsub
      fij=fi1+(j-0.5D0)*dfisub
      pxj=rc*dcos(fij)
      pyj=rc*dsin(fij)
      pzj=z0
      pxij=pxj-pxi
      pyij=pyj-pyi
      pzij=pzj-pzi
      rijsq=pxij*pxij+pyij*pyij+pzij*pzij
      rij=dsqrt(rijsq)
      rij3=rijsq*rij
      aij=aij+(pxij*uxi+pyij*uyi+pzij*uzi) *arel*drsub*dfisub/rij3
    enddo f1l0h6
  enddo q1e3h4
  if (dbc_) then
    if (isnan(aij)) stop "error in intwll, aij is NaN"
  endif
  end subroutine intwll

  end subroutine matrix_
 
  !----------------------------------------------------------------------
  ! save amx_ and indx_
  !
  ! This is the counterpoint method to 'readam'.  It saves a digest
  ! of of the input parameters critical to defining the matrix.
  ! These are the protein geometry parameters, the patch integration
  ! grid parameters and the permittivity constants. Then saves the
  ! 'amx_' matrix itself.
  subroutine writam_(fngamx) 
  implicit none
  character(*), intent(in) :: fngamx
  integer :: istat, i,j ! loop indices
  ! check variables are:
  ! patch data: npatch_, dxf_,dxw_,nsub0_,
  ! geom data: zl,rl,rlvest(),rlcurv(),epsw_,epspr_
  write(unit=fidlog,fmt=*)"Saving inverted 'A' matrix"
  open(unit=fidamx,file=fngamx,action='write',status='replace',iostat=istat)
    if (istat.ne.0) stop "Unable to open file to save the 'A' matrix"
    write(unit=fidamx,fmt='(I4)')npatch_
    write(unit=fidamx,fmt=*)  
    x2t9z8: do i=1,npatch_
      write(unit=fidamx,fmt='(I4)')indx_(i)
      s0j2d6: do j=1,npatch_
        write(unit=fidamx,fmt='(I4,1X,I4,1X,E26.18)')i,j,amx_(i,j)
      enddo s0j2d6
      write(unit=fidamx,fmt=*)  
    enddo x2t9z8
  close(unit=fidamx)
  end subroutine writam_

  !----------------------------------------------------------------------
  ! Write patch data
  !
  subroutine writpc_(fngpch)
  implicit none
  character(*), intent(in) :: fngpch
  integer :: istat, ii

  open(unit=fidpch,file=fngpch,action='write',status='replace',iostat=istat)
  if (istat.ne.0) stop 'Unable to open file to save patch geometry.'
  write(unit=fidpch,fmt='(A)')'# index xcoord  ycoord  zcoord area xnorm ynorm znorm deps_'
  write(unit=fidpch,fmt='(A)')'# ordinal ang     ang     ang    ang2 ang   ang   ang   1/permittivity'
  do ii=1,npatch_
    write(unit=fidpch,fmt='(I4,1X,F16.10,7(2x,F16.10))')ii,prx_(ii),&
&     pry_(ii),prz_(ii),parea_(ii),pux_(ii),puy_(ii),puz_(ii),deps_(ii)
  enddo
  close(unit=fidpch)
  end subroutine writpc_

end module patch


